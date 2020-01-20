#include <sdbusplus/bus.hpp>

#include <map>
#include <string>

namespace attn
{

/**
 * @brief Add entry to the attention handler event log
 */
void logEntry(const std::string& i_message,
              const std::map<std::string, std::string>& i_additional)
{

    // Get access to logging interface and method for creating log entries
    auto bus    = sdbusplus::bus::new_default_system();
    auto method = bus.new_method_call(
        "xyz.openbmc_project.Logging", "/zyz/openbmc_project/logging",
        "xyz.openbmc_project.Logging.Create", "Create");

    // Append log entry message, severity level, additional data
    method.append(i_message, "xyz.openbmc_project.Logging.Entry.Level.Error",
                  i_additional);

    // Add event to log
    auto reply = bus.call(method);

    return;
}

} // namespace attn
