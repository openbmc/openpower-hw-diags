#pragma once

namespace attn
{

// number of seconds to wait for power fault handling
constexpr int POWER_FAULT_WAIT = 10;

/** @brief Attention handler return codes */
enum ReturnCodes
{
    RC_SUCCESS        = 0,
    RC_NOT_HANDLED    = 1,
    RC_ANALYZER_ERROR = 2,
    RC_CFAM_ERROR     = 3,
    RC_DBUS_ERROR     = 4
};

/** @brief Code seciton for error reporing */
enum class AttnSection
{
    reserved        = 0x0000,
    attnHandler     = 0x0100,
    tiHandler       = 0x0200,
    handlePhypTi    = 0x0300,
    handleHbTi      = 0x0400,
    addHbStatusRegs = 0x0500,
    attnLogging     = 0x0600
};

/** @brief Attention handler error reason codes */
enum AttnCodes
{
    ATTN_NO_ERROR    = 0,
    ATTN_INFO_NULL   = 1,
    ATTN_PDBG_CFAM   = 2,
    ATTN_PDBG_SCOM   = 3,
    ATTN_INVALID_KEY = 4
};

/**
 * @brief Traces some regs for hostboot
 *
 * When we receive a Checkstop or special Attention Term Immediate,
 * hostboot wants some regs added to the error log. We will do this
 * by tracing them and then the traces will get added to the error
 * log later.
 *
 * @return nothing
 */
void addHbStatusRegs();

/**
 * @brief Check for recoverable errors present
 *
 * @return true if any recoverable errors are present, else false
 */
bool recoverableErrors();

/**
 * @brief sleep for n-seconds
 *
 * @param[in] seconds number of seconds to sleep
 */
void sleepSeconds(const unsigned int seconds);

} // namespace attn
