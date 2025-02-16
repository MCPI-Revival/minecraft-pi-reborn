#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "SDL_keysym.h"
#include "SDL_events.h"
#include "SDL_syswm.h"
#include "SDL_version.h"

int media_SDL_Init(uint32_t flags);
int media_SDL_PollEvent(SDL_Event *event);
int media_SDL_PushEvent(const SDL_Event *event);
void media_SDL_WM_SetCaption(const char *title, const char *icon);

typedef enum {
    SDL_GRAB_QUERY = -1,
    SDL_GRAB_OFF = 0,
    SDL_GRAB_ON = 1,
    SDL_GRAB_FULLSCREEN
} SDL_GrabMode;
SDL_GrabMode media_SDL_WM_GrabInput(SDL_GrabMode mode);

#define SDL_QUERY (-1)
#define SDL_IGNORE 0
#define SDL_DISABLE 0
#define SDL_ENABLE 1
int media_SDL_ShowCursor(int toggle);

#ifdef __cplusplus
}
#endif
