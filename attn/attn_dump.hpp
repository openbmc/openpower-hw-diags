#pragma once

namespace attn
{

/**
 * Request a dump from the dump manager
 *
 * Request a dump from the dump manager and register a monitor for observing
 * the dump progress.
 *
 * @param logId The id of the event log associated with this dump request
 */
void requestDump(const uint32_t logId);

} // namespace attn
