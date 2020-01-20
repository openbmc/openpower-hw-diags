#pragma once

#include <map>
#include <string>

namespace attn
{

/** @brief Add entry to attention handler event log
 *
 * The log message will consist a message and additional data. The additional
 * data can be parsed by other software to create additional log types.
 *
 * @param i_message Message for the log entry
 * @param i_additional Additional information for the log entry
 *
 */
void logEntry(const std::string& i_message,
              const std::map<std::string, std::string>& i_additional);

} // namespace attn
