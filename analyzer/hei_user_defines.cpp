/**
 * @file These are the implementations of the user interfaces declared
 *       in hei_user_defines.hpp
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

namespace libhei
{

// parse and print
void hei_inf(char* format, ...)
{
    va_list args;

    printf("I> ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

// parse and print
void hei_err(char* format, ...)
{
    va_list args;

    printf("E> ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

// print and assert
void hei_assert(char* condition)
{
    printf("Assert! (%s)\n", condition);
    assert(0);
}

} // namespace libhei
