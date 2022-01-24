#pragma once

#include <array>

namespace AthiVegam::Input
{
    /** Adopted from SDL */
    enum class KeyCode
    {
        AV_FIRST_KEY = 4,
        AV_KEY_A = AV_FIRST_KEY,
        AV_KEY_B,
        AV_KEY_C,
        AV_KEY_D,
        AV_KEY_E,
        AV_KEY_F,
        AV_KEY_G,
        AV_KEY_H,
        AV_KEY_I,
        AV_KEY_J,
        AV_KEY_K,
        AV_KEY_L,
        AV_KEY_M,
        AV_KEY_N,
        AV_KEY_O,
        AV_KEY_P,
        AV_KEY_Q,
        AV_KEY_R,
        AV_KEY_S,
        AV_KEY_T,
        AV_KEY_U,
        AV_KEY_V,
        AV_KEY_W,
        AV_KEY_X,
        AV_KEY_Y,
        AV_KEY_Z,
        AV_KEY_1,
        AV_KEY_2,
        AV_KEY_3,
        AV_KEY_4,
        AV_KEY_5,
        AV_KEY_6,
        AV_KEY_7,
        AV_KEY_8,
        AV_KEY_9,
        AV_KEY_0,
        AV_KEY_RETURN,
        AV_KEY_ESCAPE,
        AV_KEY_BACKSPACE,
        AV_KEY_TAB,
        AV_KEY_SPACE,
        AV_KEY_MINUS,
        AV_KEY_EQUALS,
        AV_KEY_LEFTBRACKET,
        AV_KEY_RIGHTBRACKET,
        AV_KEY_BACKSLASH,
        AV_KEY_NONUSHASH,
        AV_KEY_SEMICOLON,
        AV_KEY_APOSTROPHE,
        AV_KEY_GRAVE,
        AV_KEY_COMMA,
        AV_KEY_PERIOD,
        AV_KEY_SLASH,
        AV_KEY_CAPSLOCK,
        AV_KEY_F1,
        AV_KEY_F2,
        AV_KEY_F3,
        AV_KEY_F4,
        AV_KEY_F5,
        AV_KEY_F6,
        AV_KEY_F7,
        AV_KEY_F8,
        AV_KEY_F9,
        AV_KEY_F10,
        AV_KEY_F11,
        AV_KEY_F12,
        AV_KEY_PRINTSCREEN,
        AV_KEY_SCROLLLOCK,
        AV_KEY_PAUSE,
        AV_KEY_INSERT,
        AV_KEY_HOME,
        AV_KEY_PAGEUP,
        AV_KEY_DELETE,
        AV_KEY_END,
        AV_KEY_PAGEDOWN,
        AV_KEY_RIGHT,
        AV_KEY_LEFT,
        AV_KEY_DOWN,
        AV_KEY_UP,
        AV_KEY_NUMLOCKCLEAR,
        AV_KEY_KP_DIVIDE,
        AV_KEY_KP_MULTIPLY,
        AV_KEY_KP_MINUS,
        AV_KEY_KP_PLUS,
        AV_KEY_KP_ENTER,
        AV_KEY_KP_1,
        AV_KEY_KP_2,
        AV_KEY_KP_3,
        AV_KEY_KP_4,
        AV_KEY_KP_5,
        AV_KEY_KP_6,
        AV_KEY_KP_7,
        AV_KEY_KP_8,
        AV_KEY_KP_9,
        AV_KEY_KP_0,
        AV_KEY_KP_PERIOD,
        AV_KEY_NONUSBACKSLASH,
        AV_KEY_APPLICATION,
        AV_KEY_POWER,
        AV_KEY_KP_EQUALS,
        AV_KEY_F13,
        AV_KEY_F14,
        AV_KEY_F15,
        AV_KEY_F16,
        AV_KEY_F17,
        AV_KEY_F18,
        AV_KEY_F19,
        AV_KEY_F20,
        AV_KEY_F21,
        AV_KEY_F22,
        AV_KEY_F23,
        AV_KEY_F24,
        AV_KEY_EXECUTE,
        AV_KEY_HELP,
        AV_KEY_MENU,
        AV_KEY_SELECT,
        AV_KEY_STOP,
        AV_KEY_AGAIN,
        AV_KEY_UNDO,
        AV_KEY_CUT,
        AV_KEY_COPY,
        AV_KEY_PASTE,
        AV_KEY_FIND,
        AV_KEY_MUTE,
        AV_KEY_VOLUMEUP,
        AV_KEY_VOLUMEDOWN,
        AV_KEY_KP_COMMA = 133, /* Per SDL*/
        AV_KEY_KP_EQUALSAS400,
        AV_KEY_INTERNATIONAL1,
        AV_KEY_INTERNATIONAL2,
        AV_KEY_INTERNATIONAL3,
        AV_KEY_INTERNATIONAL4,
        AV_KEY_INTERNATIONAL5,
        AV_KEY_INTERNATIONAL6,
        AV_KEY_INTERNATIONAL7,
        AV_KEY_INTERNATIONAL8,
        AV_KEY_INTERNATIONAL9,
        AV_KEY_LANG1,
        AV_KEY_LANG2,
        AV_KEY_LANG3,
        AV_KEY_LANG4,
        AV_KEY_LANG5,
        AV_KEY_LANG6,
        AV_KEY_LANG7,
        AV_KEY_LANG8,
        AV_KEY_LANG9,
        AV_KEY_ALTERASE,
        AV_KEY_SYSREQ,
        AV_KEY_CANCEL,
        AV_KEY_CLEAR,
        AV_KEY_PRIOR,
        AV_KEY_RETURN2,
        AV_KEY_SEPARATOR,
        AV_KEY_OUT,
        AV_KEY_OPER,
        AV_KEY_CLEARAGAIN,
        AV_KEY_CRSEL,
        AV_KEY_EXSEL,
        AV_KEY_KP_00 = 176 /*Per SDL*/,
        AV_KEY_KP_000,
        AV_KEY_THOUSANDSSEPARATOR,
        AV_KEY_DECIMALSEPARATOR,
        AV_KEY_CURRENCYUNIT,
        AV_KEY_CURRENCYSUBUNIT,
        AV_KEY_KP_LEFTPAREN,
        AV_KEY_KP_RIGHTPAREN,
        AV_KEY_KP_LEFTBRACE,
        AV_KEY_KP_RIGHTBRACE,
        AV_KEY_KP_TAB,
        AV_KEY_KP_BACKSPACE,
        AV_KEY_KP_A,
        AV_KEY_KP_B,
        AV_KEY_KP_C,
        AV_KEY_KP_D,
        AV_KEY_KP_E,
        AV_KEY_KP_F,
        AV_KEY_KP_XOR,
        AV_KEY_KP_POWER,
        AV_KEY_KP_PERCENT,
        AV_KEY_KP_LESS,
        AV_KEY_KP_GREATER,
        AV_KEY_KP_AMPERSAND,
        AV_KEY_KP_DBLAMPERSAND,
        AV_KEY_KP_VERTICALBAR,
        AV_KEY_KP_DBLVERTICALBAR,
        AV_KEY_KP_COLON,
        AV_KEY_KP_HASH,
        AV_KEY_KP_SPACE,
        AV_KEY_KP_AT,
        AV_KEY_KP_EXCLAM,
        AV_KEY_KP_MEMSTORE,
        AV_KEY_KP_MEMRECALL,
        AV_KEY_KP_MEMCLEAR,
        AV_KEY_KP_MEMADD,
        AV_KEY_KP_MEMSUBTRACT,
        AV_KEY_KP_MEMMULTIPLY,
        AV_KEY_KP_MEMDIVIDE,
        AV_KEY_KP_PLUSMINUS,
        AV_KEY_KP_CLEAR,
        AV_KEY_KP_CLEARENTRY,
        AV_KEY_KP_BINARY,
        AV_KEY_KP_OCTAL,
        AV_KEY_KP_DECIMAL,
        AV_KEY_KP_HEXADECIMAL,
        AV_KEY_LCTRL = 224 /*Per SDl*/,
        AV_KEY_LSHIFT,
        AV_KEY_LALT,
        AV_KEY_LGUI,
        AV_KEY_RCTRL,
        AV_KEY_RSHIFT,
        AV_KEY_RALT,
        AV_KEY_RGUI,
        AV_KEY_MODE = 257, /*Per SDL*/
        AV_KEY_AUDIONEXT,
        AV_KEY_AUDIOPREV,
        AV_KEY_AUDIOSTOP,
        AV_KEY_AUDIOPLAY,
        AV_KEY_AUDIOMUTE,
        AV_KEY_MEDIASELECT,
        AV_KEY_WWW,
        AV_KEY_MAIL,
        AV_KEY_CALCULATOR,
        AV_KEY_COMPUTER,
        AV_KEY_AC_SEARCH,
        AV_KEY_AC_HOME,
        AV_KEY_AC_BACK,
        AV_KEY_AC_FORWARD,
        AV_KEY_AC_STOP,
        AV_KEY_AC_REFRESH,
        AV_KEY_AC_BOOKMARKS,
        AV_KEY_BRIGHTNESSDOWN,
        AV_KEY_BRIGHTNESSUP,
        AV_KEY_DISPLAYSWITCH,
        AV_KEY_KBDILLUMTOGGLE,
        AV_KEY_KBDILLUMDOWN,
        AV_KEY_KBDILLUMUP,
        AV_KEY_EJECT,
        AV_KEY_SLEEP,
        AV_KEY_APP1,
        AV_KEY_APP2,
        AV_KEY_AUDIOREWIND,
        AV_KEY_AUDIOFASTFORWARD
    };

	constexpr static const int KeyCount = 287; // SDL supports 286 indices for scancodes.
	using KeyStates = std::array<bool, KeyCount>;

	class Keyboard
	{
	public:
		static void Initialize();
		static void Update();

		static bool Key(KeyCode key);
		static bool KeyDown(KeyCode key);
		static bool KeyUp(KeyCode key);

	private:
		static int x, xPrev;
		static int y, yPrev;

		static KeyStates keyStates;
		static KeyStates prevKeyStates;
	};
}
