#pragma once

namespace attn
{

/**
 * @brief Load the attention handler as a gpio monitor
 *
 * Request the attention gpio for monitoring and attach the attention handler
 * as the gpio event handler.
 *
 * @param i_vital           enable vital attention handling
 * @param i_checkstop       enable checkstop attention handling
 * @param i_terminate       enable TI attention handling
 * @param i_breakpoints     enable breakpoint attention handling
 *
 * @return 0 == success
 */
int attnDaemon(bool i_vital, bool i_checkstop, bool i_terminate,
               bool i_breakpoints);

} // namespace attn
