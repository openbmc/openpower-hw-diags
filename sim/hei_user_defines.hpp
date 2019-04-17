
#pragma once

#include <stdio.h>

#define HEI_TRAC( format, ... ) \
    printf( format "\n" __VA_OPT__(,) __VA_ARGS__ )

