#pragma once

#include <stdint.h>

/** @brief Attention handler CFAM support */
static constexpr uint32_t CFAM_PIB_STATUS_REG = 0x00001007;

static constexpr uint32_t CFAM_FSI_SYSTEM_CHECKSTOP  = 0x00000002;
static constexpr uint32_t CFAM_FSI_SPECIAL_ATTENTION = 0x00000004;
static constexpr uint32_t CFAM_FSI_RECOVERABLE_ERROR = 0x00000008;
static constexpr uint32_t CFAM_FSI_CHIPLET_INT_HOST  = 0x00000010;
static constexpr uint32_t CFAM_FSI_INT_PENDING       = 0x10000000;
static constexpr uint32_t CFAM_FSI_INT_ENABLED       = 0x20000000;
static constexpr uint32_t CFAM_FSI_SBE_ATTENTION     = 0x40000000; // sbe vital

/** Attention handler high-level PDBG functions */
int test_pdbg(void);

