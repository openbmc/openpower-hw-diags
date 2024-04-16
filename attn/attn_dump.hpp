#pragma once
#include <cstdint>

namespace attn
{

/** @brief Dump types supported by dump request */
enum class DumpType
{
    Hostboot,
    Hardware,
    SBE
};

/** @brief Structure for dump request parameters */
class DumpParameters
{
  public:
    uint32_t unitId;
    DumpType dumpType;
};

/**
 * Request a dump from the dump manager
 *
 * Request a dump from the dump manager and register a monitor for observing
 * the dump progress.
 *
 * @param i_logId        The platform log ID associated with the dump request.
 * @param dumpParameters Parameters for the dump request
 */
void requestDump(uint32_t i_logId, const DumpParameters& dumpParameters);

/**
 * Enable or disable watchdog dbus property
 *
 * This property is used in enabling/ disabling host watchdog. Providing api to
 * update the dbus property
 *
 * @param enable        The property value to be set
 */
void enableWatchdog(bool enable);

} // namespace attn
