#pragma once

#include <stdint.h>

static inline uint32_t solar_os_unicode_compose(uint32_t base, uint32_t combining)
{
    if (combining == 0x0306U) {
        if (base == 0x0418U) {
            return 0x0419U;
        }
        if (base == 0x0438U) {
            return 0x0439U;
        }
    }
    if (combining == 0x0308U) {
        if (base == 0x0415U) {
            return 0x0401U;
        }
        if (base == 0x0435U) {
            return 0x0451U;
        }
        if (base == 0x0406U) {
            return 0x0407U;
        }
        if (base == 0x0456U) {
            return 0x0457U;
        }
    }
    return 0;
}
