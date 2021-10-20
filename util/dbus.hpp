#pragma once

#include <sdbusplus/bus.hpp>

#include <string>
#include <variant>
#include <vector>

namespace util
{

namespace dbus
{

using DBusValue         = std::variant<std::string, bool, std::vector<uint8_t>,
                               std::vector<std::string>>;
using DBusProperty      = std::string;
using DBusInterface     = std::string;
using DBusService       = std::string;
using DBusPath          = std::string;
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
 * @brief Read autoRebootEnabled property via
 *
 * @return false if autoRebootEnabled policy false, else true
 */
bool autoRebootEnabled();

/** @brief Host running states for host running operations */
enum class HostRunningState
{
    Unknown,
    NotStarted,
    Started
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

} // namespace dbus

} // namespace util
