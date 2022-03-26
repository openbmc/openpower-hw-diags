//------------------------------------------------------------------------------
// IMPORTANT:
// This file will ONLY be built in CI test and should be used for any functions
// that require addition support to simulate in CI test. Any functions that will
// work out-of-the-box in CI test with use of the fake device tree should be put
// in `pdbg.cpp`.
//------------------------------------------------------------------------------

#include <assert.h>

#include <test/sim-hw-access.hpp>
#include <util/log.hpp>
#include <util/pdbg.hpp>

//------------------------------------------------------------------------------

// Using this to fake the value returned from the simulation-only version of
// util::pdbg::queryLpcTimeout().
bool g_lpcTimeout = false;

namespace util
{
namespace pdbg
{

//------------------------------------------------------------------------------

// This is the simulated version of this function.
bool queryLpcTimeout(pdbg_target* target)
{
    // Must be a processor target.
    assert(TYPE_PROC == getTrgtType(target));

    // Instead of the SBE chip-op, use the faked value.
    return g_lpcTimeout;
}

//------------------------------------------------------------------------------

int getScom(pdbg_target* i_target, uint64_t i_addr, uint64_t& o_val)
{
    assert(nullptr != i_target);
    assert(TYPE_PROC == getTrgtType(i_target) ||
           TYPE_OCMB == getTrgtType(i_target));

    int rc = sim::ScomAccess::getSingleton().get(i_target, i_addr, o_val);

    if (0 != rc)
    {
        lg2::error(
            "SCOM read failure: target={SCOM_TARGET} addr={SCOM_ADDRESS}",
            "SCOM_TARGET", getPath(i_target), "SCOM_ADDRESS",
            (lg2::hex | lg2::field64), i_addr, "SCOM_ACCESS_RC", rc);
    }

    return rc;
}

//------------------------------------------------------------------------------

int getCfam(pdbg_target* i_target, uint32_t i_addr, uint32_t& o_val)
{
    assert(nullptr != i_target);
    assert(TYPE_PROC == getTrgtType(i_target));

    int rc = sim::CfamAccess::getSingleton().get(i_target, i_addr, o_val);

    if (0 != rc)
    {
        lg2::error(
            "CFAM read failure: target={CFAM_TARGET} addr={CFAM_ADDRESS}",
            "CFAM_TARGET", getPath(i_target), "CFAM_ADDRESS",
            (lg2::hex | lg2::field32), i_addr, "CFAM_ACCESS_RC", rc);
    }

    return rc;
}

//------------------------------------------------------------------------------

} // namespace pdbg
} // namespace util
