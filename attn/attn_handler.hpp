#pragma once

namespace attn
{

/**
 * @brief The main attention handler logic
 *
 * Check each processor for active attentions of type SBE Vital (vital),
 * System Checkstop (checkstop) and Special Attention (special) and handle
 * each as follows:
 *
 * checkstop: Call hardware error analyzer
 * vital:     TBD
 * special:   Determine if the special attention is a Breakpoint (BP),
 *            Terminate Immediately (TI) or CoreCodeToSp (corecode). For each
 *            special attention type, do the following:
 *
 *            BP:          Notify Cronus
 *            TI:          Start host diagnostics mode systemd unit
 *            Corecode:    TBD
 *            Recoverable: TBD
 *
 * @param i_vital       enable vital attention handling
 * @param i_checkstop   enable checkstop attention handling
 * @param i_terminate   ebable TI attention handling
 * @param i_breakpoints enable breakpoint attention handling
 */
void attnHandler(const bool i_vital, const bool i_checkstop,
                 const bool i_terminate, const bool i_breakpoints);

} // namespace attn
