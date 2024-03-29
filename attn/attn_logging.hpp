#pragma once

#include <util/ffdc_file.hpp>

#include <cstddef> // for size_t
#include <map>
#include <string>
#include <vector>

namespace attn
{
constexpr auto pathLogging = "/xyz/openbmc_project/logging";
constexpr auto levelPelError = "xyz.openbmc_project.Logging.Entry.Level.Error";
constexpr auto levelPelInfo =
    "xyz.openbmc_project.Logging.Entry.Level.Informational";
constexpr auto eventPelTerminate = "xyz.open_power.Attn.Error.Terminate";

/** @brief Logging event types */
enum class EventType
{
    Checkstop = 0,
    Terminate = 1,
    Vital = 2,
    HwDiagsFail = 3,
    AttentionFail = 4,
    PhalSbeChipop = 5
};

/** @brief Commit special attention TI event to log */
void eventTerminate(std::map<std::string, std::string> i_additionalData,
                    char* i_tiInfoData);

/** @brief Commit SBE vital event to log
 *
 *  @param[in] severity - the PEL severity level of the event
 *
 *  @return platform event log (PEL) ID
 * */
uint32_t eventVital(std::string severity);

/** @brief Commit attention handler failure event to log */
void eventAttentionFail(int i_error);

} // namespace attn
