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

TEST(RootCauseFilter, Filter1)
{
    pdbg_targets_init(nullptr);

    // Checkstop signature on the proc
    auto proc0 = util::pdbg::getTrgt("/proc0");
    libhei::Chip procChip0{proc0, P10_20};

    // EQ_CORE_FIR[14]: ME = 0 checkstop
    libhei::Signature checkstopSig{procChip0, eqCoreFir, 0, 14,
                                   libhei::ATTN_TYPE_CHECKSTOP};

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
    isoData.addSignature(ueSig);

    RasDataParser rasData{};
    libhei::Signature rootCause;
    bool attnFound = filterRootCause(AnalysisType::SYSTEM_CHECKSTOP, isoData,
                                     rootCause, rasData);
    EXPECT_TRUE(attnFound);
    EXPECT_EQ(ueSig.toUint32(), rootCause.toUint32());
}
