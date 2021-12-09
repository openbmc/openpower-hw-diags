#include <fcntl.h>

#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include <limits>

#include "gtest/gtest.h"

TEST(PDBG, PdbgDtsTest1)
{
    const uint32_t chipId            = 0; // ID for proc0.
    const std::string expProcLocCode = "Ufcs-P0-C15";
    const std::string expPhysDevPath = "physical:sys-0/node-0/proc-0";
    const int32_t expPhys_len        = expPhysDevPath.size();
    char const* fdtFileName          = (char*)"pdbg_dts_test.dtb";

    int fdtFd = open(fdtFileName, O_RDONLY);
    if (fdtFd == -1)
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
        if (PDBG_TARGET_ENABLED !=
            pdbg_target_probe(util::pdbg::getPibTrgt(procTrgt)))
            continue;

        // Process the PROC target.
        // Display Proccessor info in the target
        uint32_t attr = std::numeric_limits<uint32_t>::max();
        pdbg_target_get_attribute(procTrgt, "ATTR_CHIP_ID", 4, 1, &attr);
        trace::inf("Chip ID: %u", attr);
        ASSERT_EQ(attr, chipId);
        ASSERT_EQ(util::pdbg::getLocationCode(procTrgt), expProcLocCode);

        // Iterate the PIB, if they exist.
        pdbg_target* pibTrgt;
        pdbg_for_each_target("pib", procTrgt, pibTrgt)
        {
            // Active PIB only.
            if (PDBG_TARGET_ENABLED != pdbg_target_probe(pibTrgt))
                continue;

            // Process the PIB.
            std::string physDevPath = util::pdbg::getPhysDevPath(pibTrgt);
            ASSERT_EQ(physDevPath.substr(0, expPhys_len), expPhysDevPath);
        }
    }
}
