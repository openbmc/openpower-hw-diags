#pragma once

#include <attn/attn_dump.hpp>

namespace analyzer
{

/**
 * @brief  Queries all chips in the host hardware for the root cause of a system
 *         checkstop attention. An unrecoverable PEL will be logged containing
 *         any necessary service actions and the associated FFDC from analysis.
 *
 * @param  o_dumpParameters After system checkstop analysis, a hardware and/or
 *                          software dump is required to gather additional
 *                          information related the the system failure. This
 *                          returned parameter will indicate which type of dump
 *                          is required and other information.
 *
 * @return True, if analysis was successful and a PEL was logged. False,
 *         otherwise. Note that a dump should not be initiated when false is
 *         returned.
 */
bool analyzeCheckstopAttn(attn::DumpParameters& o_dumpParameters);

/**
 * @brief Queries all chips in the host hardware for any active recoverable or
 *        unit checkstop attentions. If any are found, an predictive PEL will be
 *        logged containing any necessary service actions and the associated
 *        FFDC from analysis.
 * @note  The purpose of this function is to be called during analysis of a
 *        Terminate Immediate (TI) event to determine if there is a hardware
 *        attention that may have caused the failure.
 */
void analyzeRecoverableAttn();

/**
 * @brief Queries all chips in the host hardware for any active attentions. If
 *        any are found, an informational PEL will be logged containing the
 *        FFDC from analysis.
 */
void manualAnalysis();

/**
 * @brief Get error analyzer build information
 *
 * @return Pointer to build information
 */
const char* getBuildInfo();

} // namespace analyzer
