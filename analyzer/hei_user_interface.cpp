/**
 * @file These are the implementations of the user interfaces declared
 *       in hei_user_interface.hpp
 */

#include <assert.h>
#include <libpdbg.h>
#include <stdarg.h>
#include <stdio.h>

#include <hei_user_interface.hpp>

namespace libhei
{

/**
 * @brief Performs a hardware register read operation.
 *
 * @param i_chip     The pdbg_target for the register access.
 *
 * @param o_buffer   Allocated memory space for the returned contents of the
 *                   register.
 *
 * @param io_bufSize The input parameter is the maximum size of the allocated
 *                   o_buffer. The return value is the number of bytes actually
 *                   written to the buffer.
 *
 * @param i_regType  The target type types for this register access.
 *
 * @param i_address  The register address. The values is a 1, 2, 4, or 8 byte
 *                   address (right justified).
 *
 * @return false => register access was successful
 *         true  => hardware access failure
 */
bool registerRead(const Chip& i_chip, void* o_buffer, size_t& io_bufSize,
                  uint64_t i_regType, uint64_t i_address)
{
    bool accessFailure = false;

    uint64_t regVal; // register read data

    assert(nullptr != o_buffer);
    assert(0 != io_bufSize);

    // read the register (pib_read == scom-read)
    if (0 != pib_read((pdbg_target*)i_chip.getChip(), i_address, &regVal))
    {
        printf("registerRead failed\n");
        io_bufSize    = 0;
        accessFailure = true;
    }
    // TODO use regType to determine register read access size. For now return
    // as much as we can, up to 64 bits (in 1, 2, 4 or 8 byte increments).
    else
    {
        if (io_bufSize >= 8)
        {
            *(uint64_t*)o_buffer = regVal;
            io_bufSize           = 8;
        }
        else
        {
            switch (io_bufSize)
            {
                case 1:
                    *(uint8_t*)o_buffer = (uint8_t)regVal;
                    break;
                case 2:
                    *(uint16_t*)o_buffer = (uint16_t)regVal;
                    break;
                case 4:
                    *(uint32_t*)o_buffer = (uint32_t)regVal;
                    break;
                default:
                    io_bufSize    = 0;
                    accessFailure = true;
            }
        }
    }

    return accessFailure;
}

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
