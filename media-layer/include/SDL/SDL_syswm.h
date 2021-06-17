#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <X11/Xlib.h>

#include "SDL_events.h"
#include "SDL_version.h"

typedef enum {
    SDL_SYSWM_X11
} SDL_SYSWM_TYPE;

typedef struct SDL_SysWMinfo {
    SDL_version version;
    SDL_SYSWM_TYPE subsystem;
    union {
        struct {
            void *display;
            XID window;
            void (*lock_func)(void);
            void (*unlock_func)(void);
            XID fswindow;
            XID wmwindow;
            void *gfxdisplay;
        } x11;
    } info;
} SDL_SysWMinfo;

#ifdef __cplusplus
}
#endif
