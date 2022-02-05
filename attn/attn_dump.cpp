#include <attn/attn_dbus.hpp>
#include <attn/attn_dump.hpp>
#include <attn/attn_logging.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <util/trace.hpp>

namespace attn
{

/**
 *  Callback for dump request properties change signal monitor
 *
 * @param[in] i_msg         Dbus message from the dbus match infrastructure
 * @param[in] i_path        The object path we are monitoring
 * @param[out] o_inProgress Used to break out of our dbus wait loop
 * @reutn Always non-zero indicating no error, no cascading callbacks
 */
uint dumpStatusChanged(sdbusplus::message::message& i_msg, std::string i_path,
                       bool& o_inProgress)
{
    // reply (msg) will be a property change message
    std::string interface;
    std::map<std::string, std::variant<std::string, uint8_t>> property;
    i_msg.read(interface, property);

    // looking for property Status changes
    std::string propertyType = "Status";
    auto dumpStatus          = property.find(propertyType);

    if (dumpStatus != property.end())
    {
        const std::string* status =
            std::get_if<std::string>(&(dumpStatus->second));

        if ((nullptr != status) && ("xyz.openbmc_project.Common.Progress."
                                    "OperationStatus.InProgress" != *status))
        {
            // dump is done, trace some info and change in progress flag
            trace::inf(i_path.c_str());
            trace::inf(status->c_str());
            o_inProgress = false;
        }
    }

    return 1; // non-negative return code for successful callback
}

/**
 * Register a callback for dump progress status changes
 *
 * @param[in] i_path The object path of the dump to monitor
 */
void monitorDump(const std::string& i_path)
{
    bool inProgress = true; // callback will update this

    // setup the signal match rules and callback
    std::string matchInterface = "xyz.openbmc_project.Common.Progress";
    auto bus                   = sdbusplus::bus::new_system();

    std::unique_ptr<sdbusplus::bus::match_t> match =
        std::make_unique<sdbusplus::bus::match_t>(
            bus,
            sdbusplus::bus::match::rules::propertiesChanged(
                i_path.c_str(), matchInterface.c_str()),
            [&](auto& msg) {
                return dumpStatusChanged(msg, i_path, inProgress);
            });

    // wait for dump status to be completed (complete == true)
    trace::inf("dump requested (waiting)");
    while (true == inProgress)
    {
        bus.wait(0);
        bus.process_discard();
    }
    trace::inf("dump completed");
}

/** Request a dump from the dump manager */
void requestDump(uint32_t i_logId, const DumpParameters& i_dumpParameters)
{
    constexpr auto path      = "/org/openpower/dump";
    constexpr auto interface = "xyz.openbmc_project.Dump.Create";
    constexpr auto function  = "CreateDump";

    sdbusplus::message::message method;

    if (0 == dbusMethod(path, interface, function, method))
    {
        try
        {
            // dbus call arguments
            std::map<std::string, std::variant<std::string, uint64_t>>
                createParams;
            createParams["com.ibm.Dump.Create.CreateParameters.ErrorLogId"] =
                uint64_t(i_logId);
            if (DumpType::Hostboot == i_dumpParameters.dumpType)
            {
                createParams["com.ibm.Dump.Create.CreateParameters.DumpType"] =
                    "com.ibm.Dump.Create.DumpType.Hostboot";
            }
            else if (DumpType::Hardware == i_dumpParameters.dumpType)
            {
                createParams["com.ibm.Dump.Create.CreateParameters.DumpType"] =
                    "com.ibm.Dump.Create.DumpType.Hardware";
                createParams
                    ["com.ibm.Dump.Create.CreateParameters.FailingUnitId"] =
                        i_dumpParameters.unitId;
            }
            else if (DumpType::SBE == i_dumpParameters.dumpType)
            {
                createParams["com.ibm.Dump.Create.CreateParameters.DumpType"] =
                    "com.ibm.Dump.Create.DumpType.SBE";
                createParams
                    ["com.ibm.Dump.Create.CreateParameters.FailingUnitId"] =
                        i_dumpParameters.unitId;
            }
            method.append(createParams);

            // using system dbus
            auto bus      = sdbusplus::bus::new_system();
            auto response = bus.call(method);

            // reply will be type dbus::ObjectPath
            sdbusplus::message::object_path reply;
            response.read(reply);

            // monitor dump progress
            monitorDump(reply);
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            trace::err("requestDump exception");
            trace::err(e.what());
        }
    }
}

} // namespace attn
