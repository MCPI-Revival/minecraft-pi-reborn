#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "SDL_keysym.h"

#define SDL_RELEASED 0
#define SDL_PRESSED 1

typedef enum {
    SDL_NOEVENT = 0,
    SDL_ACTIVEEVENT,
    SDL_KEYDOWN,
    SDL_KEYUP,
    SDL_MOUSEMOTION,
    SDL_MOUSEBUTTONDOWN,
    SDL_MOUSEBUTTONUP,
    SDL_JOYAXISMOTION,
    SDL_JOYBALLMOTION,
    SDL_JOYHATMOTION,
    SDL_JOYBUTTONDOWN,
    SDL_JOYBUTTONUP,
    SDL_QUIT,
    SDL_SYSWMEVENT,
    SDL_EVENT_RESERVEDA,
    SDL_EVENT_RESERVEDB,
    SDL_VIDEORESIZE,
    SDL_VIDEOEXPOSE,
    SDL_EVENT_RESERVED2,
    SDL_EVENT_RESERVED3,
    SDL_EVENT_RESERVED4,
    SDL_EVENT_RESERVED5,
    SDL_EVENT_RESERVED6,
    SDL_EVENT_RESERVED7,
    SDL_USEREVENT = 24,
    SDL_NUMEVENTS = 32
} SDL_EventType;

typedef struct SDL_ActiveEvent {
    uint8_t type;
    uint8_t gain;
    uint8_t state;
} SDL_ActiveEvent;

typedef struct SDL_keysym {
    uint8_t scancode;
    SDLKey sym;
    SDLMod mod;
    uint16_t unicode;
} SDL_keysym;

typedef struct SDL_KeyboardEvent {
    uint8_t type;
    uint8_t which;
    uint8_t state;
    SDL_keysym keysym;
} SDL_KeyboardEvent;

typedef struct SDL_MouseMotionEvent {
    uint8_t type;
    uint8_t which;
    uint8_t state;
    uint16_t x, y;
    int16_t xrel;
    int16_t yrel;
} SDL_MouseMotionEvent;

#define SDL_BUTTON_LEFT 1
#define SDL_BUTTON_MIDDLE 2
#define SDL_BUTTON_RIGHT 3
#define SDL_BUTTON_WHEELUP 4
#define SDL_BUTTON_WHEELDOWN 5

typedef struct SDL_MouseButtonEvent {
    uint8_t type;
    uint8_t which;
    uint8_t button;
    uint8_t state;
    uint16_t x, y;
} SDL_MouseButtonEvent;

typedef struct SDL_JoyAxisEvent {
    uint8_t type;
    uint8_t which;
    uint8_t axis;
    int16_t value;
} SDL_JoyAxisEvent;

typedef struct SDL_JoyBallEvent {
    uint8_t type;
    uint8_t which;
    uint8_t ball;
    int16_t xrel;
    int16_t yrel;
} SDL_JoyBallEvent;

typedef struct SDL_JoyHatEvent {
    uint8_t type;
    uint8_t which;
    uint8_t hat;
    uint8_t value;
} SDL_JoyHatEvent;

typedef struct SDL_JoyButtonEvent {
    uint8_t type;
    uint8_t which;
    uint8_t button;
    uint8_t state;
} SDL_JoyButtonEvent;

typedef struct SDL_ResizeEvent {
    uint8_t type;
    int32_t w;
    int32_t h;
} SDL_ResizeEvent;

typedef struct SDL_ExposeEvent {
    uint8_t type;
} SDL_ExposeEvent;

typedef struct SDL_QuitEvent {
    uint8_t type;
} SDL_QuitEvent;

typedef struct SDL_UserEvent {
    uint8_t type;
    int32_t code;
    uint32_t data1;
    uint32_t data2;
} SDL_UserEvent;

typedef struct SDL_SysWMEvent {
    uint8_t type;
    uint32_t msg;
} SDL_SysWMEvent;

typedef union SDL_Event {
    uint8_t type;
    SDL_ActiveEvent active;
    SDL_KeyboardEvent key;
    SDL_MouseMotionEvent motion;
    SDL_MouseButtonEvent button;
    SDL_JoyAxisEvent jaxis;
    SDL_JoyBallEvent jball;
    SDL_JoyHatEvent jhat;
    SDL_JoyButtonEvent jbutton;
    SDL_ResizeEvent resize;
    SDL_ExposeEvent expose;
    SDL_QuitEvent quit;
    SDL_UserEvent user;
    SDL_SysWMEvent syswm;
} SDL_Event;

#ifdef __cplusplus
}
#endif
