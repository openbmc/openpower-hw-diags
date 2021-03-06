#include <attn/attn_common.hpp>
#include <attn/attn_dbus.hpp>
#include <attn/attn_handler.hpp>
#include <attn/attn_logging.hpp>
#include <attn/pel/pel_common.hpp>
#include <attn/ti_handler.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>

#include <iomanip>
#include <iostream>

namespace attn
{

/**
 * @brief Determine if this is a HB or PHYP TI event
 *
 * Use the TI info data area to determine if this is either a HB or a PHYP
 * TI event then handle the event.
 *
 * @param i_tiDataArea pointer to the TI info data
 */
int tiHandler(TiDataArea* i_tiDataArea)
{
    int rc = RC_SUCCESS;

    // check TI data area if it is available
    if (nullptr != i_tiDataArea)
    {
        // HB v. PHYP TI logic: Only hosboot will fill in hbTerminateType
        // and it will be non-zero. Only hostboot will fill out source and
        // it it will be non-zero. Only PHYP will fill in srcFormat and it
        // will be non-zero.
        if ((0 == i_tiDataArea->hbTerminateType) &&
            (0 == i_tiDataArea->source) && (0 != i_tiDataArea->srcFormat))
        {
            handlePhypTi(i_tiDataArea);
        }
        else
        {
            handleHbTi(i_tiDataArea);
        }
    }
    else
    {
        // TI data was not available This should not happen since we provide
        // a default TI info in the case where get TI info was not successful.
        eventAttentionFail((int)AttnSection::tiHandler | ATTN_INFO_NULL);
        rc = RC_NOT_HANDLED;
    }

    return rc;
}

/**
 * @brief Handle a PHYP terminate immediate special attention
 *
 * The TI info data area will contain information pertaining to the TI
 * condition. We will wither quiesce the host or initiate a MPIPL depending
 * depending on the auto reboot configuration. We will also create a PEL which
 * will contain the TI info data and FFDC data captured in the system journal.
 *
 * @param i_tiDataArea pointer to TI information filled in by hostboot
 */
void handlePhypTi(TiDataArea* i_tiDataArea)
{
    trace<level::INFO>("PHYP TI");

    // gather additional data for PEL
    std::map<std::string, std::string> tiAdditionalData;

    if (nullptr != i_tiDataArea)
    {
        parsePhypOpalTiInfo(tiAdditionalData, i_tiDataArea);

        tiAdditionalData["Subsystem"] =
            std::to_string(static_cast<uint8_t>(pel::SubsystemID::hypervisor));

        // Copy all ascii src chars to additional data
        char srcChar[33]; // 32 ascii chars + null term
        memcpy(srcChar, &(i_tiDataArea->asciiData0), 32);
        srcChar[32]                  = 0;
        tiAdditionalData["SrcAscii"] = std::string{srcChar};

        // TI event
        eventTerminate(tiAdditionalData, (char*)i_tiDataArea);
    }
    else
    {
        // TI data was not available This should not happen since we provide
        // a default TI info in the case where get TI info was not successful.
        eventAttentionFail((int)AttnSection::handlePhypTi | ATTN_INFO_NULL);
    }

    // We are finished creating the event log entries so transition host to
    // the required state.
    if (autoRebootEnabled())
    {
        // If autoreboot is enabled we will start crash (mpipl) mode target
        transitionHost(HostState::Crash);
    }
    else
    {
        // If autoreboot is disabled we will quiesce the host
        transitionHost(HostState::Quiesce);
    }
}

/**
 * @brief Handle a hostboot terminate immediate special attention
 *
 * The TI info data area will contain information pertaining to the TI
 * condition. The course of action to take regarding the host state will
 * depend on the contents of the TI info data area. We will also create a
 * PEL containing the TI info data and FFDC data captured in the system
 * journal.
 *
 * @param i_tiDataArea pointer to TI information filled in by hostboot
 */
void handleHbTi(TiDataArea* i_tiDataArea)
{
    trace<level::INFO>("HB TI");

    bool hbDumpRequested = true; // HB dump is common case
    bool generatePel     = true; // assume PEL will be created
    bool terminateHost   = true; // transition host state

    // handle specific hostboot reason codes
    if (nullptr != i_tiDataArea)
    {
        std::stringstream ss; // stream object for tracing
        std::string strobj;   // string object for tracing

        switch (i_tiDataArea->hbTerminateType)
        {
            case TI_WITH_PLID:
            case TI_WITH_EID:

                // trace this value
                ss.str(std::string()); // empty the stream
                ss.clear();            // clear the stream
                ss << "TI with PLID/EID: " << std::hex << std::showbase
                   << std::setw(8) << std::setfill('0')
                   << be32toh(i_tiDataArea->asciiData1);
                strobj = ss.str();
                trace<level::INFO>(strobj.c_str());

                // see if HB dump is requested
                if (0 == i_tiDataArea->hbDumpFlag)
                {
                    hbDumpRequested = false; // no HB dump requested
                }
                break;
            case TI_WITH_SRC:
                // Reason code is byte 2 and 3 of 4 byte srcWord12HbWord0
                uint16_t reasonCode = be32toh(i_tiDataArea->srcWord12HbWord0);

                // trace this value
                ss.str(std::string()); // empty the stream
                ss.clear();            // clear the stream
                ss << "TI with SRC: " << std::hex << std::showbase
                   << std::setw(4) << std::setfill('0') << (int)reasonCode;
                strobj = ss.str();
                trace<level::INFO>(strobj.c_str());

                switch (reasonCode)
                {
                    case HB_SRC_SHUTDOWN_REQUEST:
                        trace<level::INFO>("shutdown request");
                        generatePel     = false;
                        hbDumpRequested = false;
                        break;
                    case HB_SRC_KEY_TRANSITION:
                        // Note: Should never see this so lets leave
                        // hbDumpRequested == true so we can figure out why
                        // we are here.
                        trace<level::INFO>("key transition");
                        terminateHost = false;
                        break;
                    case HB_SRC_INSUFFICIENT_HW:
                        trace<level::INFO>("insufficient hardware");
                        break;
                    case HB_SRC_TPM_FAIL:
                        trace<level::INFO>("TPM fail");
                        break;
                    case HB_SRC_ROM_VERIFY:
                        trace<level::INFO>("ROM verify");
                        break;
                    case HB_SRC_EXT_MISMATCH:
                        trace<level::INFO>("EXT mismatch");
                        break;
                    case HB_SRC_ECC_UE:
                        trace<level::INFO>("ECC UE");
                        break;
                    case HB_SRC_UNSUPPORTED_MODE:
                        trace<level::INFO>("unsupported mode");
                        break;
                    case HB_SRC_UNSUPPORTED_SFCRANGE:
                        trace<level::INFO>("unsupported SFC range");
                        break;
                    case HB_SRC_PARTITION_TABLE:
                        trace<level::INFO>("partition table invalid");
                        break;
                    case HB_SRC_UNSUPPORTED_HARDWARE:
                        trace<level::INFO>("unsupported hardware");
                        break;
                    case HB_SRC_PNOR_CORRUPTION:
                        trace<level::INFO>("PNOR corruption");
                        break;
                    default:
                        trace<level::INFO>("reason: other");
                }

                break; // case TI_WITH_SRC
        }
    }

    if (true == generatePel)
    {
        if (nullptr != i_tiDataArea)
        {
            // gather additional data for PEL
            std::map<std::string, std::string> tiAdditionalData;

            parseHbTiInfo(tiAdditionalData, i_tiDataArea);

            tiAdditionalData["Subsystem"] = std::to_string(
                static_cast<uint8_t>(pel::SubsystemID::hostboot));

            // Translate hex src value to ascii. This results in an 8
            // character SRC (hostboot SRC is 32 bits)
            std::stringstream src;
            src << std::setw(8) << std::setfill('0') << std::uppercase
                << std::hex << be32toh(i_tiDataArea->srcWord12HbWord0);
            tiAdditionalData["SrcAscii"] = src.str();

            // Request dump after generating event log?
            tiAdditionalData["Dump"] =
                (true == hbDumpRequested) ? "true" : "false";

            // Generate event log
            eventTerminate(tiAdditionalData, (char*)i_tiDataArea);
        }
        else
        {
            // TI data was not available This should not happen.
            eventAttentionFail((int)AttnSection::handleHbTi | ATTN_INFO_NULL);
        }
    }

    if (true == terminateHost)
    {
        transitionHost(HostState::Quiesce);
    }
}

/** @brief Parse the TI info data area into map as PHYP/OPAL data */
void parsePhypOpalTiInfo(std::map<std::string, std::string>& i_map,
                         TiDataArea* i_tiDataArea)
{
    if (nullptr == i_tiDataArea)
    {
        return;
    }

    std::stringstream ss;

    ss << "0x00 TI Area Valid:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->tiAreaValid << ":";
    ss << "0x01 Command:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->command << ":";
    ss << "0x02 Num. Data Bytes:" << std::setw(4) << std::setfill('0')
       << std::hex << be16toh(i_tiDataArea->numDataBytes) << ":";
    ss << "0x04 Reserved:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->reserved1 << ":";
    ss << "0x06 HWDump Type:" << std::setw(4) << std::setfill('0') << std::hex
       << be16toh(i_tiDataArea->hardwareDumpType) << ":";
    ss << "0x08 SRC Format:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->srcFormat << ":";
    ss << "0x09 SRC Flags:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->srcFlags << ":";
    ss << "0x0a Num. ASCII Words:" << std::setw(2) << std::setfill('0')
       << std::hex << (int)i_tiDataArea->numAsciiWords << ":";
    ss << "0x0b Num. Hex Words:" << std::setw(2) << std::setfill('0')
       << std::hex << (int)i_tiDataArea->numHexWords << ":";
    ss << "0x0e Length of SRC:" << std::setw(4) << std::setfill('0') << std::hex
       << be16toh(i_tiDataArea->lenSrc) << ":";
    ss << "0x10 SRC Word 12:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord12HbWord0) << ":";
    ss << "0x14 SRC Word 13:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord13HbWord2) << ":";
    ss << "0x18 SRC Word 14:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord14HbWord3) << ":";
    ss << "0x1c SRC Word 15:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord15HbWord4) << ":";
    ss << "0x20 SRC Word 16:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord16HbWord5) << ":";
    ss << "0x24 SRC Word 17:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord17HbWord6) << ":";
    ss << "0x28 SRC Word 18:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord18HbWord7) << ":";
    ss << "0x2c SRC Word 19:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord19HbWord8) << ":";
    ss << "0x30 ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData0) << ":";
    ss << "0x34 ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData1) << ":";
    ss << "0x38 ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData2) << ":";
    ss << "0x3c ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData3) << ":";
    ss << "0x40 ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData4) << ":";
    ss << "0x44 ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData5) << ":";
    ss << "0x48 ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData6) << ":";
    ss << "0x4c ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData7) << ":";
    ss << "0x50 Location:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->location << ":";
    ss << "0x51 Code Sections:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->codeSection << ":";
    ss << "0x52 Additional Size:" << std::setw(2) << std::setfill('0')
       << std::hex << (int)i_tiDataArea->additionalSize << ":";
    ss << "0x53 Additional Data:" << std::setw(2) << std::setfill('0')
       << std::hex << (int)i_tiDataArea->andData;

    std::string key, value;
    char delim = ':';

    while (std::getline(ss, key, delim))
    {
        std::getline(ss, value, delim);
        i_map[key] = value;
    }
}

/** @brief Parse the TI info data area into map as hostboot data */
void parseHbTiInfo(std::map<std::string, std::string>& i_map,
                   TiDataArea* i_tiDataArea)
{
    if (nullptr == i_tiDataArea)
    {
        return;
    }

    std::stringstream ss;

    ss << "0x00 TI Area Valid:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->tiAreaValid << ":";
    ss << "0x04 Reserved:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->reserved1 << ":";
    ss << "0x05 HB_Term. Type:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->hbTerminateType << ":";
    ss << "0x0c HB Dump Flag:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->hbDumpFlag << ":";
    ss << "0x0d Source:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->source << ":";
    ss << "0x10 HB Word 0:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord12HbWord0) << ":";
    ss << "0x14 HB Word 2:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord13HbWord2) << ":";
    ss << "0x18 HB Word 3:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord14HbWord3) << ":";
    ss << "0x1c HB Word 4:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord15HbWord4) << ":";
    ss << "0x20 HB Word 5:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord16HbWord5) << ":";
    ss << "0x24 HB Word 6:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord17HbWord6) << ":";
    ss << "0x28 HB Word 7:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord18HbWord7) << ":";
    ss << "0x2c HB Word 8:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord19HbWord8) << ":";
    ss << "0x30 error_data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData0) << ":";
    ss << "0x34 EID:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData1);

    std::string key, value;
    char delim = ':';

    while (std::getline(ss, key, delim))
    {
        std::getline(ss, value, delim);
        i_map[key] = value;
    }
}

/** @brief Read state of autoreboot propertyi via dbus */
bool autoRebootEnabled()
{
    // Use dbus get-property interface to read the autoreboot property
    auto bus = sdbusplus::bus::new_system();
    auto method =
        bus.new_method_call("xyz.openbmc_project.Settings",
                            "/xyz/openbmc_project/control/host0/auto_reboot",
                            "org.freedesktop.DBus.Properties", "Get");

    method.append("xyz.openbmc_project.Control.Boot.RebootPolicy",
                  "AutoReboot");

    bool autoReboot = false; // assume autoreboot attribute not available

    try
    {
        auto reply = bus.call(method);

        std::variant<bool> result;
        reply.read(result);
        autoReboot = std::get<bool>(result);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        trace<level::INFO>("autoRebootEnbabled exception");
        std::string traceMsg = std::string(e.what(), maxTraceLen);
        trace<level::ERROR>(traceMsg.c_str());
    }

    return autoReboot;
}

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
            trace<level::INFO>(i_path.c_str());
            trace<level::INFO>((*status).c_str());
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
    trace<level::INFO>("hbdump requested");
    while (true == inProgress)
    {
        bus.wait(0);
        bus.process_discard();
    }
    trace<level::INFO>("hbdump completed");
}

/** Request a dump from the dump manager */
void requestDump(const uint32_t logId)
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
            std::map<std::string, std::string> createParams;
            createParams["com.ibm.Dump.Create.CreateParameters.DumpType"] =
                "com.ibm.Dump.Create.DumpType.Hostboot";
            createParams["com.ibm.Dump.Create.CreateParameters.ErrorLogId"] =
                std::to_string(logId);
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
            trace<level::ERROR>("requestDump exception");
            std::string traceMsg = std::string(e.what(), maxTraceLen);
            trace<level::ERROR>(traceMsg.c_str());
        }
    }
}

} // namespace attn
