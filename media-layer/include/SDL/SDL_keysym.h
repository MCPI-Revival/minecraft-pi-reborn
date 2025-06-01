#pragma once

typedef enum {
    SDLK_UNKNOWN = 0,
    SDLK_BACKSPACE = 8,
    SDLK_TAB = 9,
    SDLK_RETURN = 13,
    SDLK_ESCAPE = 27,
    SDLK_SPACE = 32,
    SDLK_0 = 48,
    SDLK_1 = 49,
    SDLK_2 = 50,
    SDLK_3 = 51,
    SDLK_4 = 52,
    SDLK_5 = 53,
    SDLK_6 = 54,
    SDLK_7 = 55,
    SDLK_8 = 56,
    SDLK_9 = 57,
    SDLK_a = 97,
    SDLK_d = 100,
    SDLK_e = 101,
    SDLK_q = 113,
    SDLK_s = 115,
    SDLK_t = 116,
    SDLK_w = 119,
    SDLK_DELETE = 127,
    SDLK_UP = 273,
    SDLK_DOWN = 274,
    SDLK_RIGHT = 275,
    SDLK_LEFT = 276,
    SDLK_F1 = 282,
    SDLK_F2 = 283,
    SDLK_F3 = 284,
    SDLK_F5 = 286,
    SDLK_F11 = 292,
    SDLK_F12 = 293,
    SDLK_RSHIFT = 303,
    SDLK_LSHIFT = 304
} SDLKey;

typedef enum {
    KMOD_NONE = 0x0,
    KMOD_LSHIFT = 0x1,
    KMOD_RSHIFT = 0x2,
    KMOD_LCTRL = 0x40,
    KMOD_RCTRL = 0x80,
    KMOD_LALT = 0x100,
    KMOD_RALT = 0x200
} SDLMod;

#define KMOD_SHIFT (KMOD_LSHIFT | KMOD_RSHIFT)
#define KMOD_CTRL (KMOD_LCTRL | KMOD_RCTRL)
#define KMOD_ALT (KMOD_LALT | KMOD_RALT)
