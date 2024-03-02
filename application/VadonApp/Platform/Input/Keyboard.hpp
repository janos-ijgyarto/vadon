#ifndef VADONAPP_PLATFORM_INPUT_KEYBOARD_HPP
#define VADONAPP_PLATFORM_INPUT_KEYBOARD_HPP
#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
namespace VadonApp::Platform
{
    enum class KeyCode
    {
        UNKNOWN,
        RETURN,
        ESCAPE,
        BACKSPACE,
        SPACE,
        KEY_0,
        KEY_1,
        KEY_2,
        KEY_3,
        KEY_4,
        KEY_5,
        KEY_6,
        KEY_7,
        KEY_8,
        KEY_9,
        BACKQUOTE,
        KEY_a,
        KEY_b,
        KEY_c,
        KEY_d,
        KEY_e,
        KEY_f,
        KEY_g,
        KEY_h,
        KEY_i,
        KEY_j,
        KEY_k,
        KEY_l,
        KEY_m,
        KEY_n,
        KEY_o,
        KEY_p,
        KEY_q,
        KEY_r,
        KEY_s,
        KEY_t,
        KEY_u,
        KEY_v,
        KEY_w,
        KEY_x,
        KEY_y,
        KEY_z,
        INSERT,
        HOME,
        PAGE_UP,
        DELETE_KEY, // NOTE: needed suffix to avoid conflict with macro
        END,
        PAGE_DOWN,
        RIGHT,
        LEFT,
        DOWN,
        UP,
        LEFT_CTRL,
        LEFT_SHIFT
    };

    enum class KeyModifiers
    {
        NONE = 0,
        LEFT_SHIFT = 1 << 0,
        RIGHT_SHIFT = 1 << 1,
        LEFT_CTRL = 1 << 2,
        RIGHT_CTRL = 1 << 3,
        LEFT_ALT = 1 << 4,
        RIGHT_ALT = 1 << 5,
        LEFT_GUI = 1 << 6,
        RIGHT_GUI = 1 << 7,
        NUM_LOCK = 1 << 8,
        CAPS_LOCK = 1 << 9,
        MODE = 1 << 10,
        SCROLL_LOCK = 1 << 11,

        SHIFT = LEFT_SHIFT | RIGHT_SHIFT,
        CTRL = LEFT_CTRL | RIGHT_CTRL,
        ALT = LEFT_ALT | RIGHT_ALT,
        GUI = LEFT_GUI | RIGHT_GUI
    };
}

namespace Vadon::Utilities
{
    template<>
    struct EnableEnumBitwiseOperators<VadonApp::Platform::KeyModifiers> : public std::true_type
    {

    };
}
#endif