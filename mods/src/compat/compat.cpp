#include <unistd.h>
#include <csignal>
#include <cerrno>

#include <mods/compat/compat.h>
#include <mods/screenshot/screenshot.h>
#include <mods/init/init.h>
#include "compat-internal.h"

#include <libreborn/libreborn.h>

#include <SDL/SDL.h>
#include <media-layer/core.h>

#include <mods/input/input.h>
#include <mods/sign/sign.h>

// Custom Title
HOOK(media_SDL_WM_SetCaption, void, (__attribute__((unused)) const char *title, const char *icon)) {
    real_media_SDL_WM_SetCaption()(MCPI_APP_TITLE, icon);
}

// Mouse Cursor Is Always Invisible In Vanilla MCPI
// Because In Vanilla MCPI, The GPU Overlay Covered The Normal Mouse Cursor
HOOK(media_SDL_ShowCursor, int, (int toggle)) {
    return real_media_SDL_ShowCursor()(toggle == SDL_QUERY ? SDL_QUERY : SDL_DISABLE);
}

// Intercept SDL Events
HOOK(media_SDL_PollEvent, int, (SDL_Event *event)) {
    // In Server Mode, Exit Requests Are Handled In src/server/server.cpp
    // Check If Exit Is Requested
    if (!reborn_is_server() && compat_check_exit_requested()) {
        // Send SDL_QUIT
        SDL_Event new_event;
        new_event.type = SDL_QUIT;
        media_SDL_PushEvent(&new_event);
    }

    // Poll Events
    int ret = real_media_SDL_PollEvent()(event);

    // Handle Events
    if (ret == 1 && event != nullptr) {
        bool handled = false;
        switch (event->type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                // Track Control Key
                bool is_ctrl = (event->key.keysym.mod & KMOD_CTRL) != 0;
                if (event->type == SDL_KEYUP) {
                    is_ctrl = false;
                }
                input_set_is_ctrl(is_ctrl);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                // Track Right-Click State
                if (event->button.button == SDL_BUTTON_RIGHT) {
                    input_set_is_right_click(event->button.state != SDL_RELEASED);
                }
                break;
            }
            case SDL_USEREVENT: {
                // SDL_UserEvent Is Never Used In MCPI, So It Is Repurposed For Character Events
                if (event->user.code == USER_EVENT_CHARACTER) {
                    sign_key_press((char) event->user.data1);
                    handled = true;
                }
                break;
            }
        }
        if (handled) {
            // Event Was Handled
            return media_SDL_PollEvent(event);
        }
    }

    return ret;
}

// Exit Handler
static void exit_handler(__attribute__((unused)) int data) {
    // Request Exit
    compat_request_exit();
}
void init_compat() {
    // Install Signal Handlers
    struct sigaction act_sigint = {};
    act_sigint.sa_flags = SA_RESTART;
    act_sigint.sa_handler = &exit_handler;
    sigaction(SIGINT, &act_sigint, nullptr);
    struct sigaction act_sigterm = {};
    act_sigterm.sa_flags = SA_RESTART;
    act_sigterm.sa_handler = &exit_handler;
    sigaction(SIGTERM, &act_sigterm, nullptr);
    // Patches
    _patch_egl_calls();
    _patch_x11_calls();
    _patch_bcm_host_calls();
    _patch_sdl_calls();
}

// Store Exit Requests
static int exit_requested = 0;
int compat_check_exit_requested() {
    if (exit_requested) {
        exit_requested = 0;
        return 1;
    } else {
        return 0;
    }
}
void compat_request_exit() {
    // Request
    exit_requested = 1;
}
