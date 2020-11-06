#pragma once

#include <attn/attn_config.hpp>

namespace attn
{

/** @brief Attention handler return codes */
enum ReturnCodes
{
    RC_SUCCESS = 0,
    RC_NOT_HANDLED,
    RC_ANALYZER_ERROR,
    RC_CFAM_ERROR
};

/** @brief Attention global status bits */
constexpr uint32_t SBE_ATTN       = 0x00000002;
constexpr uint32_t CHECKSTOP_ATTN = 0x40000000;
constexpr uint32_t SPECIAL_ATTN   = 0x20000000;

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
 *
 * @param i_config pointer to attention handler configuration object
 */
void attnHandler(Config* i_config);

/**
 * @brief Determine if attention is active and not masked
 *
 * Determine whether an attention needs to be handled and trace details of
 * attention type and whether it is masked or not.
 *
 * @param i_val attention status register
 * @param i_mask attention true mask register
 * @param i_attn attention type
 * @param i_proc processor associated with registers
 *
 * @return true if attention is active and not masked, otherwise false
 */
bool activeAttn(uint32_t i_val, uint32_t i_mask, uint32_t i_attn,
                uint32_t i_proc);

} // namespace attn
