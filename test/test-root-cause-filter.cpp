#include <stdio.h>

#include <analyzer/analyzer_main.hpp>
#include <analyzer/plugins/plugin.hpp>
#include <analyzer/ras-data/ras-data-parser.hpp>
#include <hei_util.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include "gtest/gtest.h"

namespace analyzer
{
// Forward reference of filterRootCause
bool filterRootCause(AnalysisType i_type,
                     const libhei::IsolationData& i_isoData,
                     libhei::Signature& o_rootCause,
                     const RasDataParser& i_rasData);
} // namespace analyzer

using namespace analyzer;

static const auto eqCoreFir = static_cast<libhei::NodeId_t>(
    libhei::hash<libhei::NodeId_t>("EQ_CORE_FIR"));

static const auto rdfFir =
    static_cast<libhei::NodeId_t>(libhei::hash<libhei::NodeId_t>("RDFFIR"));

static const auto mc_dstl_fir = static_cast<libhei::NodeId_t>(
    libhei::hash<libhei::NodeId_t>("MC_DSTL_FIR"));

TEST(RootCauseFilter, Filter1)
{
    pdbg_targets_init(nullptr);

    RasDataParser rasData{};

    // Test 1: Test a checkstop with a UE root cause on an OCMB

    // Checkstop signature on the proc
    auto proc0 = util::pdbg::getTrgt("/proc0");
    libhei::Chip procChip0{proc0, P10_20};

    // EQ_CORE_FIR[14]: ME = 0 checkstop
    libhei::Signature checkstopSig{procChip0, eqCoreFir, 0, 14,
                                   libhei::ATTN_TYPE_CHECKSTOP};

    // MC_DSTL_FIR[1]: AFU initiated Recoverable Attn on Subchannel A
    libhei::Signature reAttnSig{procChip0, mc_dstl_fir, 0, 1,
                                libhei::ATTN_TYPE_RECOVERABLE};

    // Root cause signature on the ocmb
    auto ocmb0 =
        util::pdbg::getTrgt("proc0/pib/perv12/mc0/mi0/mcc0/omi0/ocmb0");
    libhei::Chip ocmbChip0{ocmb0, EXPLORER_20};

    // RDFFIR[14]: Mainline read UE
    libhei::Signature ueSig{ocmbChip0, rdfFir, 0, 14,
                            libhei::ATTN_TYPE_RECOVERABLE};

    // Add the signatures to the isolation data
    libhei::IsolationData isoData{};
    isoData.addSignature(checkstopSig);
    isoData.addSignature(reAttnSig);
    isoData.addSignature(ueSig);

    libhei::Signature rootCause;
    bool attnFound = filterRootCause(AnalysisType::SYSTEM_CHECKSTOP, isoData,
                                     rootCause, rasData);
    EXPECT_TRUE(attnFound);
    EXPECT_EQ(ueSig.toUint32(), rootCause.toUint32());

    // Test 2: Test a checkstop with an unknown RE attn on an OCMB

    // Add the signatures to the isolation data
    isoData.flush();
    isoData.addSignature(checkstopSig);
    isoData.addSignature(reAttnSig);

    attnFound = filterRootCause(AnalysisType::SYSTEM_CHECKSTOP, isoData,
                                rootCause, rasData);
    EXPECT_TRUE(attnFound);
    EXPECT_EQ(reAttnSig.toUint32(), rootCause.toUint32());

    // Test 3: Test a checkstop with an unknown UCS attn on an OCMB

    // MC_DSTL_FIR[0]: AFU initiated Checkstop on Subchannel A
    libhei::Signature ucsAttnSig{procChip0, mc_dstl_fir, 0, 0,
                                 libhei::ATTN_TYPE_UNIT_CS};

    isoData.flush();
    isoData.addSignature(checkstopSig);
    isoData.addSignature(ucsAttnSig);

    attnFound = filterRootCause(AnalysisType::SYSTEM_CHECKSTOP, isoData,
                                rootCause, rasData);
    EXPECT_TRUE(attnFound);
    EXPECT_EQ(ucsAttnSig.toUint32(), rootCause.toUint32());

    // Test 4: Test a checkstop with a non-root cause recoverable from an OCMB

    // RDFFIR[42]: SCOM recoverable register parity error
    libhei::Signature reSig{ocmbChip0, rdfFir, 0, 42,
                            libhei::ATTN_TYPE_RECOVERABLE};

    isoData.flush();
    isoData.addSignature(checkstopSig);
    isoData.addSignature(reAttnSig);
    isoData.addSignature(reSig);

    attnFound = filterRootCause(AnalysisType::SYSTEM_CHECKSTOP, isoData,
                                rootCause, rasData);
    EXPECT_TRUE(attnFound);
    EXPECT_EQ(checkstopSig.toUint32(), rootCause.toUint32());
}
