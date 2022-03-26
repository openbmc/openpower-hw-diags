//------------------------------------------------------------------------------
// IMPORTANT:
// This file will NOT be built in CI test and should be used for any functions
// that require addition support to simulate in CI test. Any functions that will
// work out-of-the-box in CI test with use of the fake device tree should be put
// in `pdbg.cpp`.
//------------------------------------------------------------------------------

#include <assert.h>

extern "C"
{
#include <libpdbg_sbe.h>
}

#include <util/log.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

using namespace analyzer;

namespace util
{

namespace pdbg
{

//------------------------------------------------------------------------------

bool queryLpcTimeout(pdbg_target* target)
{
    // Must be a processor target.
    assert(TYPE_PROC == getTrgtType(target));

    uint32_t result = 0;
    if (0 != sbe_lpc_timeout(util::pdbg::getPibTrgt(target), &result))
    {
        trace::err("sbe_lpc_timeout() failed: target=%s", getPath(target));
        result = 0; // just in case
    }

    // 0 if no timeout, 1 if LPC timeout occurred.
    return (0 != result);
}

//------------------------------------------------------------------------------

int getScom(pdbg_target* i_target, uint64_t i_addr, uint64_t& o_val)
{
    assert(nullptr != i_target);

    int rc = 0;

    auto targetType = getTrgtType(i_target);

    if (TYPE_PROC == targetType)
    {
        rc = pib_read(getPibTrgt(i_target), i_addr, &o_val);
    }
    else if (TYPE_OCMB == targetType)
    {
        rc = ocmb_getscom(i_target, i_addr, &o_val);
    }
    else
    {
        throw std::logic_error("Invalid type for SCOM operation: target=" +
                               std::string{getPath(i_target)});
    }

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

    int rc = fsi_read(getFsiTrgt(i_target), i_addr, &o_val);

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
