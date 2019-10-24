#include <libpdbg.h>
#include <stdint.h>

#pragma once

/**
 * @brief  The main attention handler logic
 *
 * Check each processor for active attentions of type SBE Attention (vital),
 * System Checkstop (checkstop) and Special Attention (special) and handle
 * each as follows:
 *
 * vital:     TBD
 * checkstop: Notify the hardware diagnostics application which will ... TBD
 * special:   Determine if the special attention is a Breakpoint (BP),
 *            Terminate Immediately (TI) or CoreCodeToSp (corecode). For each
 *            special attention type, do the following:
 *
 *            BP: Clear the attention status and notify Cronus
 *            TI: Write associated memory dump to a file
 *            Corecode: Clear attention status.
 */
void attnHandler();


/**
 * @brief Handle pending vital attention
 *
 * @param i_target FSI target
 *
 * @return non-zero = error
 */
int handleVital(pdbg_target * i_target);


/**
 * @brief Handle pending checkstop attention
 *
 * @param i_target FSI target
 *
 * @return non-zero = error
 */
int handleCheckstop(pdbg_target *i_target);


/**
 * @brief Handle recoverable error
 *
 * @param i_target FSI target
 *
 * @return non-zero = error
 */
int handleRecoverable(pdbg_target *i_target);


/**
 * @brief Send processor, core and thread to Cronus
 *
 * @param i_proc   Processor number with Special attention
 * @param i_core   Core number with special attention
 * @param i_thread Thread number with special attention
 */
void notifyCronus(uint32_t proc, uint32_t core, uint32_t thread);


/**
 * @brief Scom read to a specific core
 *
 * @param i_target  FSI target
 * @param i_core    Core we are targeting
 * @param i_address Scom address
 * @param o_data    Data read by Scom
 *
 * @return non-zero = null
 */
int scomReadCore(pdbg_target *i_target, uint32_t i_core,
                 uint64_t i_address, uint64_t &o_data);


/**
 * @brief Scom write to a specific core
 *
 * @param i_target  FSI target
 * @param i_core    Core we are targeting
 * @param i_address Scom address
 * @param i_data    Data to write by Scom
 *
 * @return non-zero = error
 */
int scomWriteCore(pdbg_target *i_target, uint32_t i_core,
                  uint64_t i_address, uint64_t i_data);


/**
 * @brief Read CFAM of a specific processor
 *
 * @param i_target  FSI target
 * @param i_address CFAM address
 * @param o_data    CFAM data read
 *
 * @return non-zero = error
 */
int cfamReadProc(pdbg_target *i_target, uint32_t i_address, uint32_t &o_data);


/**
 * @brief Write CFAM of a specific processor
 *
 * @param i_target  FSI target
 * @param i_address CFAM address
 * @param i_data    CFAM data to write
 *
 * @return non-zero = error
 */
int cfamWriteProc(pdbg_target *i_target, uint32_t i_address, uint32_t i_data);
