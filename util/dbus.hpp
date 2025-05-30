#pragma once

#include <sdbusplus/bus.hpp>
#include <util/ffdc_file.hpp>
#include <util/trace.hpp>

#include <string>
#include <variant>
#include <vector>

namespace util
{
namespace dbus
{
using DBusValue = std::variant<std::string, bool, std::vector<uint8_t>,
                               std::vector<std::string>, int32_t>;
using DBusProperty = std::string;
using DBusInterface = std::string;
using DBusService = std::string;
using DBusPath = std::string;
using DBusInterfaceList = std::vector<DBusInterface>;
using DBusSubTree =
    std::map<DBusPath, std::map<DBusService, DBusInterfaceList>>;

/**
 * Find the dbus object path and service that implements the given interface
 *
 * @param[in]   i_interface Interface to search for
 * @param[out]  o_path      Path of dbus object implementing the interface
 * @param[out]  o_service   Service implementing the dbus object path
 * @return      non-zero on error
 */
int find(const std::string& i_interface, std::string& o_path,
         std::string& o_service);

/**
 * Find the dbus service that implements the given dbus object and interface
 *
 * @param[in]   i_interface Interface that maps to the service
 * @param[in]   i_path      Path that maps to the service
 * @param[out]  o_service   Service implementing the dbus object and interface
 * @return      non-zer on error
 */
int findService(const std::string& i_interface, const std::string& i_path,
                std::string& o_service);

/**
 * Read a property from a dbus object interface
 *
 * @param[in]   i_interface Interface implementing the property
 * @param[in]   i_path      Path of the dbus object
 * @param[in]   i_service   Service implementing the dbus object and interface
 * @param[in]   i_property  Property to read
 * @return      non-zero on error
 */
int getProperty(const std::string& i_interface, const std::string& i_path,
                const std::string& i_service, const std::string& i_property,
                DBusValue& o_response);

template <typename T>
void setProperty(const std::string& service, const std::string& object,
                 const std::string& interface, const std::string& propertyName,
                 const std::variant<T>& propertyValue)
{
    try
    {
        auto bus = sdbusplus::bus::new_default();
        auto method =
            bus.new_method_call(service.c_str(), object.c_str(),
                                "org.freedesktop.DBus.Properties", "Set");
        method.append(interface);
        method.append(propertyName);
        method.append(propertyValue);

        bus.call(method);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        trace::err("util::dbus::setProperty exception");
        std::string traceMsg = std::string(e.what());
        trace::err(traceMsg.c_str());
    }
}

/**
 * Get the IBM compatible names defined for this system
 *
 * @return     A vector of strings containing the system names
 */
std::vector<std::string> systemNames();

/** @brief Host transition states for host transition operations */
enum class HostState
{
    Quiesce,
    Diagnostic,
    Crash
};

/**
 * @brief Transition the host state
 *
 * We will transition the host state by starting the appropriate dbus target.
 *
 * @param i_hostState the state to transition the host to
 */
void transitionHost(const HostState i_hostState);

/**
 * @brief Read autoRebootEnabled property
 *
 * @return false if autoRebootEnabled policy false, else true
 */
bool autoRebootEnabled();

/** @brief Host running states for host running operations */
enum class HostRunningState
{
    Unknown,
    NotStarted,
    Started,
    Stopping
};

/**
 * Get the host running state
 *
 * Use host boot progress to determine if a host has been started. If host
 * boot progress can not be determined then host state will be unknown.
 *
 * @return HostType == "Unknown", "Started or "NotStarted"
 */
HostRunningState hostRunningState();

/**
 * @brief Read dumpPolicyEnabled property
 *
 * @return false if dumpPolicyEnabled property is false, else true
 */
bool dumpPolicyEnabled();

/**
 * Create a PEL
 *
 * The additional data provided in the map will be placed in a user data
 * section of the PEL and may additionally contain key words to trigger
 * certain behaviors by the backend logging code. Each set of data described
 * in the vector of ffdc data will be placed in additional user data
 * sections. Note that the PID of the caller will be added to the additional
 * data map with key "_PID".
 *
 * @param  i_message - the event type
 * @param  i_severity - the severity level
 * @param  io_additional - map of additional data
 * @param  i_ffdc - vector of ffdc data
 * @return Platform log id or 0 if error
 */
uint32_t createPel(const std::string& i_message, const std::string& i_severity,
                   std::map<std::string, std::string>& io_additional,
                   const std::vector<FFDCTuple>& i_ffdc);

/** @brief Machine ID definitions */
enum class MachineType
{
    Rainier_2S4U,
    Rainier_2S2U,
    Rainier_1S4U,
    Rainier_1S2U,
    Everest,
    Bonnell,
    BlueRidge_2S4U,
    BlueRidge_2S2U,
    BlueRidge_1S4U,
    Fuji,
    Balcones,
};

/**
 * @brief Read the System IM keyword to get the machine type
 *
 * @return An enum representing the machine type
 */
MachineType getMachineType();

/** @brief Get list of state sensor PDRs
 *
 *  @param[out] pdrList - list of PDRs
 *  @param[in] stateSetId - ID of the state set of interest
 *
 *  @return true if successful otherwise false
 */
bool getStateSensorPdrs(std::vector<std::vector<uint8_t>>& pdrList,
                        uint16_t stateSetId);

/** @brief Get list of state effecter PDRs
 *
 *  @param[out] pdrList -  list of PDRs
 *  @param[in] stateSetId - ID of the state set of interest
 *
 *  @return true if successful otherwise false
 */
bool getStateEffecterPdrs(std::vector<std::vector<uint8_t>>& pdrList,
                          uint16_t stateSetId);

/**
 * @brief Determine if power fault was detected
 *
 * @return true if power fault or unknown, false otherwise
 */
bool powerFault();

} // namespace dbus
} // namespace util
