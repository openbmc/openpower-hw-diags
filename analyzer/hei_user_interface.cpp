/**
 * @file These are the implementations of the user interfaces declared
 *       in hei_user_interface.hpp
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include <hei_user_interface.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

ReturnCode registerRead(const Chip& i_chip, void* o_buffer, size_t& io_bufSize,
                        uint64_t i_regType, uint64_t i_address)
{
    ReturnCode rc{};

    assert(nullptr != o_buffer);
    assert(0 != io_bufSize);

    // TODO need real register read code
    printf("registerRead not implemented\n");

    rc = RC_SUCCESS;

    return rc;
}

//------------------------------------------------------------------------------

#ifndef __HEI_READ_ONLY

ReturnCode registerWrite(const Chip& i_chip, void* i_buffer, size_t& io_bufSize,
                         uint64_t i_regType, uint64_t i_address)
{
    ReturnCode rc{};

    assert(nullptr != i_buffer);
    assert(0 != io_bufSize);

    // TODO need real register write code
    printf("registerWrite not implemented\n");

    rc = RC_SUCCESS;

    return rc;
}

#endif

//------------------------------------------------------------------------------

void hei_inf(char* format, ...)
{
    va_list args;

    printf("I> ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

//------------------------------------------------------------------------------

void hei_err(char* format, ...)
{
    va_list args;

    printf("E> ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

} // namespace libhei
