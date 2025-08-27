#ifndef SYSTEM_H
#define SYSTEM_H

#include <SDL2/SDL.h>
#include "../util/Types.h"

typedef enum MicroKey
{
    MICRO_KEY_UNKNOWN = 0,

    /**
     *  \name Usage page 0x07
     *
     *  These values are from usage page 0x07 (USB keyboard page).
     */
    /* @{ */

    MICRO_KEY_A = 4,
    MICRO_KEY_B = 5,
    MICRO_KEY_C = 6,
    MICRO_KEY_D = 7,
    MICRO_KEY_E = 8,
    MICRO_KEY_F = 9,
    MICRO_KEY_G = 10,
    MICRO_KEY_H = 11,
    MICRO_KEY_I = 12,
    MICRO_KEY_J = 13,
    MICRO_KEY_K = 14,
    MICRO_KEY_L = 15,
    MICRO_KEY_M = 16,
    MICRO_KEY_N = 17,
    MICRO_KEY_O = 18,
    MICRO_KEY_P = 19,
    MICRO_KEY_Q = 20,
    MICRO_KEY_R = 21,
    MICRO_KEY_S = 22,
    MICRO_KEY_T = 23,
    MICRO_KEY_U = 24,
    MICRO_KEY_V = 25,
    MICRO_KEY_W = 26,
    MICRO_KEY_X = 27,
    MICRO_KEY_Y = 28,
    MICRO_KEY_Z = 29,

    MICRO_KEY_1 = 30,
    MICRO_KEY_2 = 31,
    MICRO_KEY_3 = 32,
    MICRO_KEY_4 = 33,
    MICRO_KEY_5 = 34,
    MICRO_KEY_6 = 35,
    MICRO_KEY_7 = 36,
    MICRO_KEY_8 = 37,
    MICRO_KEY_9 = 38,
    MICRO_KEY_0 = 39,

    MICRO_KEY_RETURN = 40,
    MICRO_KEY_ESCAPE = 41,
    MICRO_KEY_BACKSPACE = 42,
    MICRO_KEY_TAB = 43,
    MICRO_KEY_SPACE = 44,

    MICRO_KEY_MINUS = 45,
    MICRO_KEY_EQUALS = 46,
    MICRO_KEY_LEFTBRACKET = 47,
    MICRO_KEY_RIGHTBRACKET = 48,
    MICRO_KEY_BACKSLASH = 49, /**< Located at the lower left of the return
                                  *   key on ISO keyboards and at the right end
                                  *   of the QWERTY row on ANSI keyboards.
                                  *   Produces REVERSE SOLIDUS (backslash) and
                                  *   VERTICAL LINE in a US layout, REVERSE
                                  *   SOLIDUS and VERTICAL LINE in a UK Mac
                                  *   layout, NUMBER SIGN and TILDE in a UK
                                  *   Windows layout, DOLLAR SIGN and POUND SIGN
                                  *   in a Swiss German layout, NUMBER SIGN and
                                  *   APOSTROPHE in a German layout, GRAVE
                                  *   ACCENT and POUND SIGN in a French Mac
                                  *   layout, and ASTERISK and MICRO SIGN in a
                                  *   French Windows layout.
                                  */
    MICRO_KEY_NONUSHASH = 50, /**< ISO USB keyboards actually use this code
                                  *   instead of 49 for the same key, but all
                                  *   OSes I've seen treat the two codes
                                  *   identically. So, as an implementor, unless
                                  *   your keyboard generates both of those
                                  *   codes and your OS treats them differently,
                                  *   you should generate MICRO_KEY_BACKSLASH
                                  *   instead of this code. As a user, you
                                  *   should not rely on this code because SDL
                                  *   will never generate it with most (all?)
                                  *   keyboards.
                                  */
    MICRO_KEY_SEMICOLON = 51,
    MICRO_KEY_APOSTROPHE = 52,
    MICRO_KEY_GRAVE = 53, /**< Located in the top left corner (on both ANSI
                              *   and ISO keyboards). Produces GRAVE ACCENT and
                              *   TILDE in a US Windows layout and in US and UK
                              *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                              *   and NOT SIGN in a UK Windows layout, SECTION
                              *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                              *   layouts on ISO keyboards, SECTION SIGN and
                              *   DEGREE SIGN in a Swiss German layout (Mac:
                              *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                              *   DEGREE SIGN in a German layout (Mac: only on
                              *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                              *   French Windows layout, COMMERCIAL AT and
                              *   NUMBER SIGN in a French Mac layout on ISO
                              *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                              *   SIGN in a Swiss German, German, or French Mac
                              *   layout on ANSI keyboards.
                              */
    MICRO_KEY_COMMA = 54,
    MICRO_KEY_PERIOD = 55,
    MICRO_KEY_SLASH = 56,

    MICRO_KEY_CAPSLOCK = 57,

    MICRO_KEY_F1 = 58,
    MICRO_KEY_F2 = 59,
    MICRO_KEY_F3 = 60,
    MICRO_KEY_F4 = 61,
    MICRO_KEY_F5 = 62,
    MICRO_KEY_F6 = 63,
    MICRO_KEY_F7 = 64,
    MICRO_KEY_F8 = 65,
    MICRO_KEY_F9 = 66,
    MICRO_KEY_F10 = 67,
    MICRO_KEY_F11 = 68,
    MICRO_KEY_F12 = 69,

    MICRO_KEY_PRINTSCREEN = 70,
    MICRO_KEY_SCROLLLOCK = 71,
    MICRO_KEY_PAUSE = 72,
    MICRO_KEY_INSERT = 73, /**< insert on PC, help on some Mac keyboards (but
                                   does send code 73, not 117) */
    MICRO_KEY_HOME = 74,
    MICRO_KEY_PAGEUP = 75,
    MICRO_KEY_DELETE = 76,
    MICRO_KEY_END = 77,
    MICRO_KEY_PAGEDOWN = 78,
    MICRO_KEY_RIGHT = 79,
    MICRO_KEY_LEFT = 80,
    MICRO_KEY_DOWN = 81,
    MICRO_KEY_UP = 82,

    MICRO_KEY_NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
                                     */
    MICRO_KEY_KP_DIVIDE = 84,
    MICRO_KEY_KP_MULTIPLY = 85,
    MICRO_KEY_KP_MINUS = 86,
    MICRO_KEY_KP_PLUS = 87,
    MICRO_KEY_KP_ENTER = 88,
    MICRO_KEY_KP_1 = 89,
    MICRO_KEY_KP_2 = 90,
    MICRO_KEY_KP_3 = 91,
    MICRO_KEY_KP_4 = 92,
    MICRO_KEY_KP_5 = 93,
    MICRO_KEY_KP_6 = 94,
    MICRO_KEY_KP_7 = 95,
    MICRO_KEY_KP_8 = 96,
    MICRO_KEY_KP_9 = 97,
    MICRO_KEY_KP_0 = 98,
    MICRO_KEY_KP_PERIOD = 99,

    MICRO_KEY_NONUSBACKSLASH = 100, /**< This is the additional key that ISO
                                        *   keyboards have over ANSI ones,
                                        *   located between left shift and Y.
                                        *   Produces GRAVE ACCENT and TILDE in a
                                        *   US or UK Mac layout, REVERSE SOLIDUS
                                        *   (backslash) and VERTICAL LINE in a
                                        *   US or UK Windows layout, and
                                        *   LESS-THAN SIGN and GREATER-THAN SIGN
                                        *   in a Swiss German, German, or French
                                        *   layout. */
    MICRO_KEY_APPLICATION = 101, /**< windows contextual menu, compose */
    MICRO_KEY_POWER = 102, /**< The USB document says this is a status flag,
                               *   not a physical key - but some Mac keyboards
                               *   do have a power key. */
    MICRO_KEY_KP_EQUALS = 103,
    MICRO_KEY_F13 = 104,
    MICRO_KEY_F14 = 105,
    MICRO_KEY_F15 = 106,
    MICRO_KEY_F16 = 107,
    MICRO_KEY_F17 = 108,
    MICRO_KEY_F18 = 109,
    MICRO_KEY_F19 = 110,
    MICRO_KEY_F20 = 111,
    MICRO_KEY_F21 = 112,
    MICRO_KEY_F22 = 113,
    MICRO_KEY_F23 = 114,
    MICRO_KEY_F24 = 115,
    MICRO_KEY_EXECUTE = 116,
    MICRO_KEY_HELP = 117,    /**< AL Integrated Help Center */
    MICRO_KEY_MENU = 118,    /**< Menu (show menu) */
    MICRO_KEY_SELECT = 119,
    MICRO_KEY_STOP = 120,    /**< AC Stop */
    MICRO_KEY_AGAIN = 121,   /**< AC Redo/Repeat */
    MICRO_KEY_UNDO = 122,    /**< AC Undo */
    MICRO_KEY_CUT = 123,     /**< AC Cut */
    MICRO_KEY_COPY = 124,    /**< AC Copy */
    MICRO_KEY_PASTE = 125,   /**< AC Paste */
    MICRO_KEY_FIND = 126,    /**< AC Find */
    MICRO_KEY_MUTE = 127,
    MICRO_KEY_VOLUMEUP = 128,
    MICRO_KEY_VOLUMEDOWN = 129,
/* not sure whether there's a reason to enable these */
/*     MICRO_KEY_LOCKINGCAPSLOCK = 130,  */
/*     MICRO_KEY_LOCKINGNUMLOCK = 131, */
/*     MICRO_KEY_LOCKINGSCROLLLOCK = 132, */
    MICRO_KEY_KP_COMMA = 133,
    MICRO_KEY_KP_EQUALSAS400 = 134,

    MICRO_KEY_INTERNATIONAL1 = 135, /**< used on Asian keyboards, see
                                            footnotes in USB doc */
    MICRO_KEY_INTERNATIONAL2 = 136,
    MICRO_KEY_INTERNATIONAL3 = 137, /**< Yen */
    MICRO_KEY_INTERNATIONAL4 = 138,
    MICRO_KEY_INTERNATIONAL5 = 139,
    MICRO_KEY_INTERNATIONAL6 = 140,
    MICRO_KEY_INTERNATIONAL7 = 141,
    MICRO_KEY_INTERNATIONAL8 = 142,
    MICRO_KEY_INTERNATIONAL9 = 143,
    MICRO_KEY_LANG1 = 144, /**< Hangul/English toggle */
    MICRO_KEY_LANG2 = 145, /**< Hanja conversion */
    MICRO_KEY_LANG3 = 146, /**< Katakana */
    MICRO_KEY_LANG4 = 147, /**< Hiragana */
    MICRO_KEY_LANG5 = 148, /**< Zenkaku/Hankaku */
    MICRO_KEY_LANG6 = 149, /**< reserved */
    MICRO_KEY_LANG7 = 150, /**< reserved */
    MICRO_KEY_LANG8 = 151, /**< reserved */
    MICRO_KEY_LANG9 = 152, /**< reserved */

    MICRO_KEY_ALTERASE = 153,    /**< Erase-Eaze */
    MICRO_KEY_SYSREQ = 154,
    MICRO_KEY_CANCEL = 155,      /**< AC Cancel */
    MICRO_KEY_CLEAR = 156,
    MICRO_KEY_PRIOR = 157,
    MICRO_KEY_RETURN2 = 158,
    MICRO_KEY_SEPARATOR = 159,
    MICRO_KEY_OUT = 160,
    MICRO_KEY_OPER = 161,
    MICRO_KEY_CLEARAGAIN = 162,
    MICRO_KEY_CRSEL = 163,
    MICRO_KEY_EXSEL = 164,

    MICRO_KEY_KP_00 = 176,
    MICRO_KEY_KP_000 = 177,
    MICRO_KEY_THOUSANDSSEPARATOR = 178,
    MICRO_KEY_DECIMALSEPARATOR = 179,
    MICRO_KEY_CURRENCYUNIT = 180,
    MICRO_KEY_CURRENCYSUBUNIT = 181,
    MICRO_KEY_KP_LEFTPAREN = 182,
    MICRO_KEY_KP_RIGHTPAREN = 183,
    MICRO_KEY_KP_LEFTBRACE = 184,
    MICRO_KEY_KP_RIGHTBRACE = 185,
    MICRO_KEY_KP_TAB = 186,
    MICRO_KEY_KP_BACKSPACE = 187,
    MICRO_KEY_KP_A = 188,
    MICRO_KEY_KP_B = 189,
    MICRO_KEY_KP_C = 190,
    MICRO_KEY_KP_D = 191,
    MICRO_KEY_KP_E = 192,
    MICRO_KEY_KP_F = 193,
    MICRO_KEY_KP_XOR = 194,
    MICRO_KEY_KP_POWER = 195,
    MICRO_KEY_KP_PERCENT = 196,
    MICRO_KEY_KP_LESS = 197,
    MICRO_KEY_KP_GREATER = 198,
    MICRO_KEY_KP_AMPERSAND = 199,
    MICRO_KEY_KP_DBLAMPERSAND = 200,
    MICRO_KEY_KP_VERTICALBAR = 201,
    MICRO_KEY_KP_DBLVERTICALBAR = 202,
    MICRO_KEY_KP_COLON = 203,
    MICRO_KEY_KP_HASH = 204,
    MICRO_KEY_KP_SPACE = 205,
    MICRO_KEY_KP_AT = 206,
    MICRO_KEY_KP_EXCLAM = 207,
    MICRO_KEY_KP_MEMSTORE = 208,
    MICRO_KEY_KP_MEMRECALL = 209,
    MICRO_KEY_KP_MEMCLEAR = 210,
    MICRO_KEY_KP_MEMADD = 211,
    MICRO_KEY_KP_MEMSUBTRACT = 212,
    MICRO_KEY_KP_MEMMULTIPLY = 213,
    MICRO_KEY_KP_MEMDIVIDE = 214,
    MICRO_KEY_KP_PLUSMINUS = 215,
    MICRO_KEY_KP_CLEAR = 216,
    MICRO_KEY_KP_CLEARENTRY = 217,
    MICRO_KEY_KP_BINARY = 218,
    MICRO_KEY_KP_OCTAL = 219,
    MICRO_KEY_KP_DECIMAL = 220,
    MICRO_KEY_KP_HEXADECIMAL = 221,

    MICRO_KEY_LCTRL = 224,
    MICRO_KEY_LSHIFT = 225,
    MICRO_KEY_LALT = 226, /**< alt, option */
    MICRO_KEY_LGUI = 227, /**< windows, command (apple), meta */
    MICRO_KEY_RCTRL = 228,
    MICRO_KEY_RSHIFT = 229,
    MICRO_KEY_RALT = 230, /**< alt gr, option */
    MICRO_KEY_RGUI = 231, /**< windows, command (apple), meta */

    MICRO_KEY_MODE = 257,    /**< I'm not sure if this is really not covered
                                 *   by any of the above, but since there's a
                                 *   special KMOD_MODE for it I'm adding it here
                                 */

    /* @} *//* Usage page 0x07 */

    /**
     *  \name Usage page 0x0C
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     *  See https://usb.org/sites/default/files/hut1_2.pdf
     *
     *  There are way more keys in the spec than we can represent in the
     *  current scancode range, so pick the ones that commonly come up in
     *  real world usage.
     */
    /* @{ */

    MICRO_KEY_AUDIONEXT = 258,
    MICRO_KEY_AUDIOPREV = 259,
    MICRO_KEY_AUDIOSTOP = 260,
    MICRO_KEY_AUDIOPLAY = 261,
    MICRO_KEY_AUDIOMUTE = 262,
    MICRO_KEY_MEDIASELECT = 263,
    MICRO_KEY_WWW = 264,             /**< AL Internet Browser */
    MICRO_KEY_MAIL = 265,
    MICRO_KEY_CALCULATOR = 266,      /**< AL Calculator */
    MICRO_KEY_COMPUTER = 267,
    MICRO_KEY_AC_SEARCH = 268,       /**< AC Search */
    MICRO_KEY_AC_HOME = 269,         /**< AC Home */
    MICRO_KEY_AC_BACK = 270,         /**< AC Back */
    MICRO_KEY_AC_FORWARD = 271,      /**< AC Forward */
    MICRO_KEY_AC_STOP = 272,         /**< AC Stop */
    MICRO_KEY_AC_REFRESH = 273,      /**< AC Refresh */
    MICRO_KEY_AC_BOOKMARKS = 274,    /**< AC Bookmarks */

    /* @} *//* Usage page 0x0C */

    /**
     *  \name Walther keys
     *
     *  These are values that Christian Walther added (for mac keyboard?).
     */
    /* @{ */

    MICRO_KEY_BRIGHTNESSDOWN = 275,
    MICRO_KEY_BRIGHTNESSUP = 276,
    MICRO_KEY_DISPLAYSWITCH = 277, /**< display mirroring/dual display
                                           switch, video mode switch */
    MICRO_KEY_KBDILLUMTOGGLE = 278,
    MICRO_KEY_KBDILLUMDOWN = 279,
    MICRO_KEY_KBDILLUMUP = 280,
    MICRO_KEY_EJECT = 281,
    MICRO_KEY_SLEEP = 282,           /**< SC System Sleep */

    MICRO_KEY_APP1 = 283,
    MICRO_KEY_APP2 = 284,

    /* @} *//* Walther keys */

    /**
     *  \name Usage page 0x0C (additional media keys)
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    MICRO_KEY_AUDIOREWIND = 285,
    MICRO_KEY_AUDIOFASTFORWARD = 286,

    /* @} *//* Usage page 0x0C (additional media keys) */

    /**
     *  \name Mobile keys
     *
     *  These are values that are often used on mobile phones.
     */
    /* @{ */

    MICRO_KEY_SOFTLEFT = 287, /**< Usually situated below the display on phones and
                                      used as a multi-function feature key for selecting
                                      a software defined function shown on the bottom left
                                      of the display. */
    MICRO_KEY_SOFTRIGHT = 288, /**< Usually situated below the display on phones and
                                       used as a multi-function feature key for selecting
                                       a software defined function shown on the bottom right
                                       of the display. */
    MICRO_KEY_CALL = 289, /**< Used for accepting phone calls. */
    MICRO_KEY_ENDCALL = 290, /**< Used for rejecting phone calls. */

    /* @} *//* Mobile keys */

    /* Add any other keys here. */

    MICRO_NUM_KEYS = 512 /**< not a key, just marks the number of scancodes
                                 for array bounds */
} MicroKey;

int microSystemInit();
int microSystemFree();
int microSystemGetKey(MicroKey scancode);
void microSystemGetMousePos(int *x, int *y);
void microSystemSetMousePos(int x, int y);
void microSystemGetWindowSize(int *width, int *height);
void microSystemShowCursor(bool show);
void microSystemFocusWindow();
void microSystemWindowSwapBuffers();
bool microSystemIsGamepadConnected();
SDL_GameController *microSystemGetGameController();


#endif /* end of include guard: SYSTEM_H */
