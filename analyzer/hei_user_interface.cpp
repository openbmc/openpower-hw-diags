/**
 * @file These are the implementations of the user interfaces declared
 *       in hei_user_interface.hpp
 */

#include <hei_user_interface.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

ReturnCode registerRead(const Chip& i_chip, void* o_buffer, size_t& io_bufSize,
                        uint64_t i_regType, uint64_t i_address)
{
    ReturnCode rc{};

    HEI_ASSERT(nullptr != o_buffer);
    HEI_ASSERT(0 != io_bufSize);

    // TODO need real register read code
    HEI_ERR("registerRead not implemented");

    rc = RC_SUCCESS;

    return rc;
}

//------------------------------------------------------------------------------

#ifndef __HEI_READ_ONLY

ReturnCode registerWrite(const Chip& i_chip, void* i_buffer, size_t& io_bufSize,
                         uint64_t i_regType, uint64_t i_address)
{
    ReturnCode rc{};

    HEI_ASSERT(nullptr != i_buffer);
    HEI_ASSERT(0 != io_bufSize);

    // TODO need real register write code
    HEI_ERR("registerWrite not implemented");

    rc = RC_SUCCESS;

    return rc;
}

#endif

//------------------------------------------------------------------------------

} // namespace libhei
