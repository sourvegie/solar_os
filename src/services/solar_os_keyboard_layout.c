#include "solar_os_keyboard_layout.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "solar_os_input.h"
#include "solar_os_keys.h"

#define KEYBOARD_LATIN1_A_UMLAUT_UPPER 0x00c4U
#define KEYBOARD_LATIN1_O_UMLAUT_UPPER 0x00d6U
#define KEYBOARD_LATIN1_U_UMLAUT_UPPER 0x00dcU
#define KEYBOARD_LATIN1_SHARP_S 0x00dfU
#define KEYBOARD_LATIN1_A_UMLAUT_LOWER 0x00e4U
#define KEYBOARD_LATIN1_O_UMLAUT_LOWER 0x00f6U
#define KEYBOARD_LATIN1_U_UMLAUT_LOWER 0x00fcU

static const char *const keyboard_layout_names[SOLAR_OS_INPUT_KEYBOARD_LAYOUT_COUNT] = {
    [SOLAR_OS_INPUT_KEYBOARD_LAYOUT_US] = "us",
    [SOLAR_OS_INPUT_KEYBOARD_LAYOUT_DE] = "de",
    [SOLAR_OS_INPUT_KEYBOARD_LAYOUT_RU] = "ru",
};

static solar_os_keyboard_translation_t keyboard_key(uint8_t key)
{
    return (solar_os_keyboard_translation_t) {.key = key};
}

static solar_os_keyboard_translation_t keyboard_codepoint(uint32_t codepoint)
{
    if (codepoint > 0 && codepoint <= 0x7fU) {
        return keyboard_key((uint8_t)codepoint);
    }
    return (solar_os_keyboard_translation_t) {.codepoint = codepoint};
}

const char *solar_os_keyboard_layout_name(solar_os_input_keyboard_layout_t layout)
{
    if (layout >= SOLAR_OS_INPUT_KEYBOARD_LAYOUT_COUNT) {
        return "unknown";
    }
    return keyboard_layout_names[layout];
}

bool solar_os_keyboard_layout_parse(const char *name,
                                    solar_os_input_keyboard_layout_t *layout)
{
    if (name == NULL || layout == NULL) {
        return false;
    }
    for (size_t i = 0; i < SOLAR_OS_INPUT_KEYBOARD_LAYOUT_COUNT; i++) {
        if (strcmp(name, keyboard_layout_names[i]) == 0) {
            *layout = (solar_os_input_keyboard_layout_t)i;
            return true;
        }
    }
    return false;
}

static uint8_t keyboard_shifted_digit(uint16_t usage)
{
    static const uint8_t shifted[] = {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')'};
    return shifted[usage - 0x1eU];
}

static uint8_t keyboard_unshifted_digit(uint16_t usage)
{
    return usage == 0x27U ? '0' : (uint8_t)('1' + usage - 0x1eU);
}

static uint8_t keyboard_usage_to_us(uint16_t usage, bool shift, bool caps_lock)
{
    if (usage >= 0x04U && usage <= 0x1dU) {
        const bool upper = shift ^ caps_lock;
        return (uint8_t)((upper ? 'A' : 'a') + usage - 0x04U);
    }
    if (usage >= 0x1eU && usage <= 0x27U) {
        return shift ? keyboard_shifted_digit(usage) : keyboard_unshifted_digit(usage);
    }
    switch (usage) {
    case 0x28: return '\n';
    case 0x29: return SOLAR_OS_KEY_ESCAPE;
    case 0x2a: return '\b';
    case 0x2b: return '\t';
    case 0x2c: return ' ';
    case 0x2d: return shift ? '_' : '-';
    case 0x2e: return shift ? '+' : '=';
    case 0x2f: return shift ? '{' : '[';
    case 0x30: return shift ? '}' : ']';
    case 0x31: return shift ? '|' : '\\';
    case 0x32: return shift ? '~' : '#';
    case 0x33: return shift ? ':' : ';';
    case 0x34: return shift ? '"' : '\'';
    case 0x35: return shift ? '~' : '`';
    case 0x36: return shift ? '<' : ',';
    case 0x37: return shift ? '>' : '.';
    case 0x38: return shift ? '?' : '/';
    default: return 0;
    }
}

static uint32_t keyboard_usage_to_de(uint16_t usage,
                                     uint8_t modifiers,
                                     bool caps_lock)
{
    const bool shift = (modifiers & SOLAR_OS_INPUT_MOD_SHIFT) != 0;
    const bool altgr = (modifiers & SOLAR_OS_INPUT_MOD_RIGHT_ALT) != 0;
    if (altgr) {
        switch (usage) {
        case 0x2b: return '\t';
        case 0x14: return '@';
        case 0x24: return '{';
        case 0x25: return '[';
        case 0x26: return ']';
        case 0x27: return '}';
        case 0x2d: return '\\';
        case 0x30: return '~';
        case 0x64: return '|';
        default: return 0;
        }
    }
    if (usage >= 0x04U && usage <= 0x1dU) {
        uint8_t base = (uint8_t)('a' + usage - 0x04U);
        if (base == 'y') {
            base = 'z';
        } else if (base == 'z') {
            base = 'y';
        }
        return (shift ^ caps_lock) ? (uint8_t)toupper(base) : base;
    }
    if (usage >= 0x1eU && usage <= 0x27U) {
        static const uint8_t shifted[] = {'!', '"', '#', '$', '%', '&', '/', '(', ')', '='};
        return shift ? shifted[usage - 0x1eU] : keyboard_unshifted_digit(usage);
    }
    switch (usage) {
    case 0x28: return '\n';
    case 0x29: return SOLAR_OS_KEY_ESCAPE;
    case 0x2a: return '\b';
    case 0x2b: return '\t';
    case 0x2c: return ' ';
    case 0x2d: return shift ? '?' : KEYBOARD_LATIN1_SHARP_S;
    case 0x2e: return shift ? '`' : 0;
    case 0x2f: return shift ? KEYBOARD_LATIN1_U_UMLAUT_UPPER : KEYBOARD_LATIN1_U_UMLAUT_LOWER;
    case 0x30: return shift ? '*' : '+';
    case 0x31:
    case 0x32: return shift ? '\'' : '#';
    case 0x33: return shift ? KEYBOARD_LATIN1_O_UMLAUT_UPPER : KEYBOARD_LATIN1_O_UMLAUT_LOWER;
    case 0x34: return shift ? KEYBOARD_LATIN1_A_UMLAUT_UPPER : KEYBOARD_LATIN1_A_UMLAUT_LOWER;
    case 0x35: return shift ? 0 : '^';
    case 0x36: return shift ? ';' : ',';
    case 0x37: return shift ? ':' : '.';
    case 0x38: return shift ? '_' : '-';
    case 0x64: return shift ? '>' : '<';
    default: return 0;
    }
}

static bool keyboard_ru_letter_usage(uint16_t usage)
{
    return (usage >= 0x04U && usage <= 0x1dU) ||
        usage == 0x2fU || usage == 0x30U || usage == 0x33U ||
        usage == 0x34U || usage == 0x35U || usage == 0x36U || usage == 0x37U;
}

static uint32_t keyboard_usage_to_ru(uint16_t usage, bool shift, bool caps_lock)
{
    static const uint32_t lower_letters[] = {
        0x0444, 0x0438, 0x0441, 0x0432, 0x0443, 0x0430, 0x043f, 0x0440,
        0x0448, 0x043e, 0x043b, 0x0434, 0x044c, 0x0442, 0x0449, 0x0437,
        0x0439, 0x043a, 0x044b, 0x0435, 0x0433, 0x043c, 0x0446, 0x0447,
        0x043d, 0x044f,
    };
    static const uint32_t upper_letters[] = {
        0x0424, 0x0418, 0x0421, 0x0412, 0x0423, 0x0410, 0x041f, 0x0420,
        0x0428, 0x041e, 0x041b, 0x0414, 0x042c, 0x0422, 0x0429, 0x0417,
        0x0419, 0x041a, 0x042b, 0x0415, 0x0413, 0x041c, 0x0426, 0x0427,
        0x041d, 0x042f,
    };
    const bool upper = keyboard_ru_letter_usage(usage) && (shift ^ caps_lock);

    if (usage >= 0x04U && usage <= 0x1dU) {
        return upper ? upper_letters[usage - 0x04U] : lower_letters[usage - 0x04U];
    }
    if (usage >= 0x1eU && usage <= 0x27U) {
        static const uint32_t shifted[] = {'!', '"', 0x2116, ';', '%', ':', '?', '*', '(', ')'};
        return shift ? shifted[usage - 0x1eU] : keyboard_unshifted_digit(usage);
    }
    switch (usage) {
    case 0x28: return '\n';
    case 0x29: return SOLAR_OS_KEY_ESCAPE;
    case 0x2a: return '\b';
    case 0x2b: return '\t';
    case 0x2c: return ' ';
    case 0x2d: return shift ? '_' : '-';
    case 0x2e: return shift ? '+' : '=';
    case 0x2f: return upper ? 0x0425 : 0x0445;
    case 0x30: return upper ? 0x042a : 0x044a;
    case 0x31: return shift ? '/' : '\\';
    case 0x32: return 0;
    case 0x33: return upper ? 0x0416 : 0x0436;
    case 0x34: return upper ? 0x042d : 0x044d;
    case 0x35: return upper ? 0x0401 : 0x0451;
    case 0x36: return upper ? 0x0411 : 0x0431;
    case 0x37: return upper ? 0x042e : 0x044e;
    case 0x38: return shift ? ',' : '.';
    default: return 0;
    }
}

static uint8_t keyboard_usage_to_function_key(uint16_t usage)
{
    if (usage >= 0x3aU && usage <= 0x45U) {
        return (uint8_t)(SOLAR_OS_KEY_F1 + usage - 0x3aU);
    }
    return 0;
}

static uint8_t keyboard_usage_to_nav_key(uint16_t usage, uint8_t modifiers)
{
    const bool ctrl = (modifiers & SOLAR_OS_INPUT_MOD_CTRL) != 0;
    const bool shift = (modifiers & SOLAR_OS_INPUT_MOD_SHIFT) != 0;
    switch (usage) {
    case 0x4a:
        return ctrl ? (shift ? SOLAR_OS_KEY_CTRL_SHIFT_HOME : SOLAR_OS_KEY_CTRL_HOME) :
            (shift ? SOLAR_OS_KEY_SHIFT_HOME : SOLAR_OS_KEY_HOME);
    case 0x4b: return shift ? SOLAR_OS_KEY_SHIFT_PAGE_UP : SOLAR_OS_KEY_PAGE_UP;
    case 0x4c: return SOLAR_OS_KEY_DELETE;
    case 0x4d:
        return ctrl ? (shift ? SOLAR_OS_KEY_CTRL_SHIFT_END : SOLAR_OS_KEY_CTRL_END) :
            (shift ? SOLAR_OS_KEY_SHIFT_END : SOLAR_OS_KEY_END);
    case 0x4e: return shift ? SOLAR_OS_KEY_SHIFT_PAGE_DOWN : SOLAR_OS_KEY_PAGE_DOWN;
    case 0x4f:
        return ctrl ? (shift ? SOLAR_OS_KEY_CTRL_SHIFT_RIGHT : SOLAR_OS_KEY_CTRL_RIGHT) :
            (shift ? SOLAR_OS_KEY_SHIFT_RIGHT : SOLAR_OS_KEY_RIGHT);
    case 0x50:
        return ctrl ? (shift ? SOLAR_OS_KEY_CTRL_SHIFT_LEFT : SOLAR_OS_KEY_CTRL_LEFT) :
            (shift ? SOLAR_OS_KEY_SHIFT_LEFT : SOLAR_OS_KEY_LEFT);
    case 0x51:
        return ctrl ? (shift ? SOLAR_OS_KEY_CTRL_SHIFT_DOWN : SOLAR_OS_KEY_CTRL_DOWN) :
            (shift ? SOLAR_OS_KEY_SHIFT_DOWN : SOLAR_OS_KEY_DOWN);
    case 0x52:
        return ctrl ? (shift ? SOLAR_OS_KEY_CTRL_SHIFT_UP : SOLAR_OS_KEY_CTRL_UP) :
            (shift ? SOLAR_OS_KEY_SHIFT_UP : SOLAR_OS_KEY_UP);
    default: return 0;
    }
}

static uint8_t keyboard_usage_to_control(uint16_t usage,
                                         solar_os_input_keyboard_layout_t layout)
{
    if (usage >= 0x04U && usage <= 0x1dU) {
        uint8_t base = (uint8_t)('a' + usage - 0x04U);
        if (layout == SOLAR_OS_INPUT_KEYBOARD_LAYOUT_DE) {
            if (base == 'y') {
                base = 'z';
            } else if (base == 'z') {
                base = 'y';
            }
        }
        return (uint8_t)(base - 'a' + 1U);
    }
    switch (usage) {
    case 0x23: return 0x1e;
    case 0x2d: return 0x1f;
    case 0x30: return 0x1d;
    case 0x31: return 0x1c;
    default: return 0;
    }
}

solar_os_keyboard_translation_t solar_os_keyboard_layout_translate(
    solar_os_input_keyboard_layout_t layout,
    uint16_t usage,
    uint8_t modifiers,
    bool caps_lock)
{
    const bool ctrl = (modifiers & SOLAR_OS_INPUT_MOD_CTRL) != 0;
    const bool alt = (modifiers & SOLAR_OS_INPUT_MOD_ALT) != 0;
    const bool shift = (modifiers & SOLAR_OS_INPUT_MOD_SHIFT) != 0;

    if (ctrl && usage == 0x2cU) {
        return keyboard_key(SOLAR_OS_KEY_KEYBOARD_LAYOUT_TOGGLE);
    }
    if (ctrl && alt && usage == 0x4cU) {
        return keyboard_key(SOLAR_OS_KEY_APP_EXIT);
    }
    if (ctrl && !alt) {
        if (layout != SOLAR_OS_INPUT_KEYBOARD_LAYOUT_DE && usage == 0x30U) {
            return keyboard_key(SOLAR_OS_KEY_APP_EXIT);
        }
        if (usage == 0x2eU ||
            (layout == SOLAR_OS_INPUT_KEYBOARD_LAYOUT_DE && usage == 0x30U)) {
            return keyboard_key(SOLAR_OS_KEY_CTRL_PLUS);
        }
        if (layout == SOLAR_OS_INPUT_KEYBOARD_LAYOUT_DE && usage == 0x38U) {
            return keyboard_key(0x1f);
        }
    }

    uint8_t key = keyboard_usage_to_function_key(usage);
    if (key == 0) {
        key = keyboard_usage_to_nav_key(usage, modifiers);
    }
    if (key == 0 && ctrl) {
        key = keyboard_usage_to_control(usage, layout);
    }
    if (key != 0) {
        return keyboard_key(key);
    }

    if (layout == SOLAR_OS_INPUT_KEYBOARD_LAYOUT_DE) {
        return keyboard_codepoint(keyboard_usage_to_de(usage, modifiers, caps_lock));
    }
    if (layout == SOLAR_OS_INPUT_KEYBOARD_LAYOUT_RU) {
        return keyboard_codepoint(keyboard_usage_to_ru(usage, shift, caps_lock));
    }
    return keyboard_key(keyboard_usage_to_us(usage, shift, caps_lock));
}
