#pragma once

#include <map>

namespace attn
{
/**
 * @brief Add entry to the attention handler event log
 *
 * @param i_message     The message property of the event log object. For PEL
 *                      creation this also coorelates to the error name in the
 *                      messge registry.
 *
 * @param i_additional  AdditionalData property of the event log object. For
 *                      PEL creationg this can be used to proved SRC data
 *                      words 6-9.
 */
void logEntry(const std::string& i_message,
              const std::map<std::string, std::string>& i_additional);

} // namespace attn
