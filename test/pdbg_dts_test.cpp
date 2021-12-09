#include <fcntl.h>

#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include <limits>

#include "gtest/gtest.h"

TEST(PDBG, PdbgDtsTest1)
{
    const uint32_t chipId  = 0; // ID for proc0.
    const uint32_t fapiPos = 0; // FAPI Position proc0.

    pdbg_targets_init(nullptr);

    // Iterate each processor.
    pdbg_target* procTrgt;
    pdbg_for_each_class_target("proc", procTrgt)
    {
        // Active processors only.
        if (PDBG_TARGET_ENABLED !=
            pdbg_target_probe(util::pdbg::getPibTrgt(procTrgt)))
            continue;

        // Process the PROC target.
        // Display Proccessor info in the target
        uint32_t attr = std::numeric_limits<uint32_t>::max();
        pdbg_target_get_attribute(procTrgt, "ATTR_CHIP_ID", 4, 1, &attr);
        trace::inf("Chip ID: %u", attr);
        ASSERT_EQ(attr, chipId);

        attr = std::numeric_limits<uint32_t>::max();
        pdbg_target_get_attribute(procTrgt, "ATTR_FAPI_POS", 4, 1, &attr);
        trace::inf("ATTR_FAPI_POS: %u", attr);
        ASSERT_EQ(attr, fapiPos);
        break;
    }
}
