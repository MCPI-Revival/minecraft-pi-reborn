#include <SDL/SDL.h>
#include <media-layer/core.h>

#include <libreborn/util/util.h>
#include <libreborn/config.h>

#include <symbols/minecraft.h>

#include <mods/compat/compat.h>
#include <mods/input/input.h>
#include <mods/feature/feature.h>

#include "internal.h"

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
static bool enable_text_events;
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
                    if (enable_text_events) {
                        Keyboard::_inputText.push_back(char(event->user.data1));
                    }
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

// Init
void _init_compat_sdl() {
    enable_text_events = feature_has("Enable Text Input", server_disabled);
}