/**
 * @file These are implementations of the user interfaces declared in
 *       hei_user_interface.hpp
 */

#include <hei_user_interface.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

bool registerRead(const Chip& i_chip, void* o_buffer, size_t& io_bufSize,
                        uint64_t i_regType, uint64_t i_address)
{
    bool accessFailure = false;

    //    HEI_INF("registerRead(%p,%p,%lx,%lx,%lx)", i_chip.getChip(),
    //             o_buffer, io_bufSize, i_regType, i_address);

    return accessFailure;
}

//------------------------------------------------------------------------------

#ifndef __HEI_READ_ONLY

bool registerWrite(const Chip& i_chip, void* i_buffer, size_t& io_bufSize,
                         uint64_t i_regType, uint64_t i_address)
{
    bool accessFailure = false;

    //    HEI_INF("registerWrite(%p,%p,%lx,%lx,%lx)", i_chip.getChip(),
    //             i_buffer, io_bufSize, i_regType, i_address);

    return accessFailure;
}

#endif

//------------------------------------------------------------------------------

} // namespace libhei
