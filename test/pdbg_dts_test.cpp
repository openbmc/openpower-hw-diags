#include <hei_main.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include <limits>

#include "gtest/gtest.h"

TEST(PDBG, PdbgDtsTest1)
{
    uint32_t chipId       = std::numeric_limits<uint32_t>::max();
    string expProcLocCode = "Ufcs-P0-C15";
    string expPhysDevPath = "physical:sys-0/node-0/proc-0/nx-0

                            ";

        pdbg_targets_init(nullptr); // nullptr == use default

    // Iterate each processor.
    pdbg_target* procTrgt;
    pdbg_for_each_class_target("proc", procTrgt)
    {
        // Active processors only.
        if (PDBG_TARGET_ENABLED != pdbg_target_probe(getPibTrgt(procTrgt)))
            continue;

        // Process the PROC target.
        // Display Proccessor info in the target
        uint32_t attr = 0;
        pdbg_target_get_attribute(procTrgt, "ATTR_CHIP_ID", 4, 1, &attr);
        trace::inf("Chip ID: %u", attr);
        ASSERT_EQ(attr, procId);
        ASSERT_EQ(getLocationCode(chipId), expLocCode);

        // Iterate the PIB, if they exist.
        pdbg_target* pibTrgt;
        pdbg_for_each_target("pib", procTrgt, pibTrgt)
        {
            // Active PIB only.
            if (PDBG_TARGET_ENABLED != pdbg_target_probe(pibTrgt))
                continue;

            // Process the PIB.
            ASSERT_EQ(getPhysDevPath(pdbg_target* trgt), expPhysDevPath);
        }
    }
}
