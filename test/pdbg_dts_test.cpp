#include<fcntl.h>

#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include <limits>

#include "gtest/gtest.h"


TEST(PDBG, PdbgDtsTest1)
{
    uint32_t chipId;
    = 0; // ID for proc0.
    string expProcLocCode = "Ufcs-P0-C15";
    string expPhysDevPath =
        "physical:sys-0/node-0/proc-0/nx-0" char* fdtFileName =
            "pdbg_dts_test.dtb";
    int fdtFd;

    fdtFd = open(fdtFileName, O_RDONLY);
    if (fdt == -1)
    {
        trace::err("Cannot open file: %s", fdtFileName);
    }
    ASSERT_NE(fdtFd, -1);

    pdbg_targets_init(&fdtFd);

    // Iterate each processor.
    pdbg_target* procTrgt;
    pdbg_for_each_class_target("proc", procTrgt)
    {
        // Active processors only.
        if (PDBG_TARGET_ENABLED != pdbg_target_probe(getPibTrgt(procTrgt)))
            continue;

        // Process the PROC target.
        // Display Proccessor info in the target
        uint32_t attr = std::numeric_limits<uint32_t>::max();
        pdbg_target_get_attribute(procTrgt, "ATTR_CHIP_ID", 4, 1, &attr);
        trace::inf("Chip ID: %u", attr);
        ASSERT_EQ(attr, chipId);
        ASSERT_EQ(getLocationCode(procTrgt), expLocCode);

        // Iterate the PIB, if they exist.
        pdbg_target* pibTrgt;
        pdbg_for_each_target("pib", procTrgt, pibTrgt)
        {
            // Active PIB only.
            if (PDBG_TARGET_ENABLED != pdbg_target_probe(pibTrgt))
                continue;

            // Process the PIB.
            ASSERT_EQ(getPhysDevPath(pdbg_target * trgt), expPhysDevPath);
        }
    }
}
