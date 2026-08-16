#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SOLAR_OS_INPUT_KEYBOARD_LAYOUT_US,
    SOLAR_OS_INPUT_KEYBOARD_LAYOUT_DE,
    SOLAR_OS_INPUT_KEYBOARD_LAYOUT_RU,
    SOLAR_OS_INPUT_KEYBOARD_LAYOUT_COUNT,
} solar_os_input_keyboard_layout_t;

typedef struct {
    uint8_t key;
    uint32_t codepoint;
} solar_os_keyboard_translation_t;

const char *solar_os_keyboard_layout_name(solar_os_input_keyboard_layout_t layout);
bool solar_os_keyboard_layout_parse(const char *name,
                                    solar_os_input_keyboard_layout_t *layout);
solar_os_keyboard_translation_t solar_os_keyboard_layout_translate(
    solar_os_input_keyboard_layout_t layout,
    uint16_t usage,
    uint8_t modifiers,
    bool caps_lock);
