
#include <libpdbg.h>

#include <util/pdbg.hpp>
#include <util/trace.hpp>

namespace util
{

namespace pdbg
{

//------------------------------------------------------------------------------

const char* __getPath(pdbg_target* i_trgt)
{
    return pdbg_target_path(i_trgt);
}

const char* getPath(const libhei::Chip& i_chip)
{
    return __getPath((pdbg_target*)i_chip.getChip());
}

//------------------------------------------------------------------------------

uint32_t __getChipPos(pdbg_target* i_trgt)
{
    uint32_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_FAPI_POS", 4, 1, &attr);
    return attr;
}

uint32_t getChipPos(const libhei::Chip& i_chip)
{
    return __getChipPos((pdbg_target*)i_chip.getChip());
}

//------------------------------------------------------------------------------

uint32_t __getChipId(pdbg_target* i_trgt)
{
    uint32_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_CHIP_ID", 4, 1, &attr);
    return attr;
}

uint8_t __getChipEc(pdbg_target* i_trgt)
{
    uint8_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_EC", 1, 1, &attr);
    return attr;
}

uint32_t __getChipIdEc(pdbg_target* i_trgt)
{
    return ((__getChipId(i_trgt) & 0xffff) << 16) | __getChipEc(i_trgt);
}

//------------------------------------------------------------------------------

void __addChip(std::vector<libhei::Chip>& o_chips, pdbg_target* i_trgt,
               libhei::ChipType_t i_type)
{
    // Trace each chip for debug. It is important to show the type just in case
    // the model/EC does not exist. See note below.
    trace::inf("Chip found: type=0x%08" PRIx32 " chip=%s", i_type,
               __getPath(i_trgt));

    if (0 == i_type)
    {
        // There are two reasons for this:
        //  - Due to some design limitation in libpdbg, all processors are
        //    considered active, even if they do not exist.
        //  - There is a special case where the model/level attributes have not
        //    been initialized in the devtree. This is possible on the epoch IPL
        //    where an attention occurs before Hostboot is able to update the
        //    devtree information on the BMC.
        // In both cases, just ignore the chip.
    }
    else
    {
        o_chips.emplace_back(i_trgt, i_type);
    }
}

void getActiveChips(std::vector<libhei::Chip>& o_chips)
{
    o_chips.clear();

    // Iterate each processor.
    pdbg_target* procTrgt;
    pdbg_for_each_class_target("proc", procTrgt)
    {
        // Active processors only.
        if (PDBG_TARGET_ENABLED != pdbg_target_probe(procTrgt))
            continue;

        // Add the processor to the list.
        __addChip(o_chips, procTrgt, __getChipIdEc(procTrgt));

        // Iterate the connected OCMBs, if they exist.
        pdbg_target* ocmbTrgt;
        pdbg_for_each_target("ocmb", procTrgt, ocmbTrgt)
        {
            // Active OCMBs only.
            if (PDBG_TARGET_ENABLED != pdbg_target_probe(ocmbTrgt))
                continue;

            // Add the OCMB to the list.
            __addChip(o_chips, ocmbTrgt, __getChipIdEc(ocmbTrgt));
        }
    }
}

//------------------------------------------------------------------------------

} // namespace pdbg

} // namespace util
