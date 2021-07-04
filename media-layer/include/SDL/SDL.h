#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "SDL_keysym.h"
#include "SDL_events.h"
#include "SDL_syswm.h"
#include "SDL_version.h"

int SDL_Init(uint32_t flags);
int SDL_PollEvent(SDL_Event *event);
int SDL_PushEvent(SDL_Event *event);
void SDL_WM_SetCaption(const char *title, const char *icon);

typedef enum {
    SDL_GRAB_QUERY = -1,
    SDL_GRAB_OFF = 0,
    SDL_GRAB_ON = 1,
    SDL_GRAB_FULLSCREEN
} SDL_GrabMode;
SDL_GrabMode SDL_WM_GrabInput(SDL_GrabMode mode);

#define SDL_QUERY -1
#define SDL_IGNORE 0
#define SDL_DISABLE 0
#define SDL_ENABLE 1
int SDL_ShowCursor(int toggle);

void *SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags);
int SDL_GetWMInfo(SDL_SysWMinfo *info);
__attribute__((noreturn)) void SDL_Quit();

#ifdef __cplusplus
}
#endif
