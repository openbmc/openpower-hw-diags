#pragma once

#include <stdint.h>

/** @brief Attention handler CFAM support */
static const uint32_t CFAM_ISR_VAL          = 0x00001007;
static const uint32_t CFAM_ISR_CLEAR        = 0x0000100b;
static const uint32_t CFAM_ISR_MASK         = 0x0000100d;

static const uint32_t CFAM_ISR_SBE          = 0x00000002; // fixme be/le
static const uint32_t CFAM_ISR_CHKSTOP      = 0x40000000; // fixme be/le
static const uint32_t CFAM_ISR_SATTN        = 0x20000000; // fixme be/le
static const uint32_t CFAM_ISR_RECOV        = 0x10000000; // fixme be/le

/** @brief Scom register support */
static const uint32_t SCOM_GLOBAL_SATTN_REG = 0x500f001a; // broadcast
static const uint32_t SCOM_ATTN_STATUS_REG  = 0x20010a99; // core
static const uint32_t SCOM_ATTN_MASK_REG    = 0x20010a9a; // core
static const uint32_t SCOM_CORE_STATE_REG   = 0x20010ab3; // core
static const uint32_t SCOM_PROC_ISR_CLEAR   = 0x000f001a; // proc

static constexpr uint64_t SCOM_FUSED_CORE_BIT = (1ull << 63);

/* @brief Core thread status bits */
static const uint32_t INSTR_STOP_BIT        = 0;
static const uint32_t ATTN_COMPLETE_BIT     = 1;
static const uint32_t RECOV_HANDSHAKE_BIT   = 2;
static const uint32_t CORE_CODE_BIT         = 3;

/** @brief Attention handler logic */
void attnHandler();

