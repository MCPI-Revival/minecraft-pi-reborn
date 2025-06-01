#pragma once

#include <cstdint>

#include "SDL_keysym.h"

typedef enum {
    SDL_NOEVENT = 0,
    SDL_KEYDOWN = 2,
    SDL_KEYUP = 3,
    SDL_MOUSEMOTION = 4,
    SDL_MOUSEBUTTONDOWN = 5,
    SDL_MOUSEBUTTONUP = 6,
    SDL_QUIT = 12,
    SDL_USEREVENT = 24
} SDL_EventType;

#define SDL_RELEASED 0
#define SDL_PRESSED 1

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

typedef struct SDL_QuitEvent {
    uint8_t type;
} SDL_QuitEvent;

typedef struct SDL_UserEvent {
    uint8_t type;
    int32_t code;
    uint32_t data1;
    uint32_t data2;
} SDL_UserEvent;

#define SDL_EVENT_SIZE 0x14
typedef union SDL_Event {
    uint8_t type;
    SDL_KeyboardEvent key;
    SDL_MouseMotionEvent motion;
    SDL_MouseButtonEvent button;
    SDL_QuitEvent quit;
    SDL_UserEvent user;
    unsigned char __data[SDL_EVENT_SIZE];
} SDL_Event;
static_assert(sizeof(SDL_Event) == SDL_EVENT_SIZE);