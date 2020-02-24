#pragma once

/**
 * @file hei_user_defines.hpp
 * @brief The purpose of this file is to create provide defines that
 *        are needed by the hardware error isolator library (libhei)
 */

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#define HEI_INF(...)                                                           \
    {                                                                          \
        printf("HWDIAGS I> " __VA_ARGS__);                                     \
        printf("\n");                                                          \
    }

#define HEI_ERR(...)                                                           \
    {                                                                          \
        printf("HWDIAGS E> " __VA_ARGS__);                                     \
        printf("\n");                                                          \
    }

#define HEI_ASSERT(expression) assert(expression);
