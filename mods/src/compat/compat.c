#include <unistd.h>
#include <signal.h>

#include "compat.h"
#include "../init/init.h"

#ifndef MCPI_SERVER_MODE
#include <SDL/SDL.h>

#include <media-layer/core.h>
#include <libreborn/libreborn.h>

#include "../input/input.h"
#include "../sign/sign.h"
#include "../chat/chat.h"
#include "../home/home.h"

// Custom Title
HOOK(SDL_WM_SetCaption, void, (__attribute__((unused)) const char *title, const char *icon)) {
    ensure_SDL_WM_SetCaption();
    (*real_SDL_WM_SetCaption)(GUI_TITLE, icon);
}

// Mouse Cursor Is Always Invisible In Vanilla MCPI
// Because In Vanilla MCPI, The GPU Overlay Covered The Normal Mouse Cursor
HOOK(SDL_ShowCursor, int, (int toggle)) {
    ensure_SDL_ShowCursor();
    return (*real_SDL_ShowCursor)(toggle == SDL_QUERY ? SDL_QUERY : SDL_DISABLE);
}

// Intercept SDL Events
HOOK(SDL_PollEvent, int, (SDL_Event *event)) {
    // In Server Mode, Exit Requests Are Handled In src/server/server.cpp
    // Check If Exit Is Requested
    if (compat_check_exit_requested()) {
        // Send SDL_QUIT
        SDL_Event new_event;
        new_event.type = SDL_QUIT;
        SDL_PushEvent(&new_event);
    }

    // Poll Events
    ensure_SDL_PollEvent();
    int ret = (*real_SDL_PollEvent)(event);

    // Handle Events
    if (ret == 1 && event != NULL) {
        int handled = 0;

        switch (event->type) {
            case SDL_KEYDOWN: {
                // Handle Key Presses
                if (event->key.keysym.sym == SDLK_F11) {
                    media_toggle_fullscreen();
                    handled = 1;
                } else if (event->key.keysym.sym == SDLK_F2) {
                    media_take_screenshot(home_get());
                    handled = 1;
                } else if (event->key.keysym.sym == SDLK_F1) {
                    input_hide_gui();
                    handled = 1;
                } else if (event->key.keysym.sym == SDLK_F5) {
                    input_third_person();
                    handled = 1;
                } else if (event->key.keysym.sym == SDLK_t) {
                    // Only When In-Game With No Other Chat Windows Open
                    if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON && chat_get_counter() == 0) {
                        // Open Chat
                        chat_open();
                    }
                    // Mark Handled
                    handled = 1;
                } else if (event->key.keysym.sym == SDLK_ESCAPE) {
                    // Treat Escape As Back Button Press (This Fixes Issues With Signs)
                    handled = input_back();
                } else if (event->key.keysym.sym == SDLK_q) {
                    // Drop Item
                    input_drop((event->key.keysym.mod & KMOD_CTRL) != 0);
                    handled = 1;
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                // Track Right-Click State
                if (event->button.button == SDL_BUTTON_RIGHT) {
                    input_set_is_right_click(event->button.state != SDL_RELEASED);
                } else if (event->button.button == SDL_BUTTON_LEFT) {
                    input_set_is_left_click(event->button.state != SDL_RELEASED);
                }
                break;
            }
            case SDL_USEREVENT: {
                // SDL_UserEvent Is Never Used In MCPI, So It Is Repurposed For Character Events
                sign_key_press((char) event->user.code);
                handled = 1;
                break;
            }
        }

        if (handled) {
            // Event Was Handled
            return SDL_PollEvent(event);
        }
    }

    return ret;
}
#endif

// Exit Handler
static void exit_handler(__attribute__((unused)) int data) {
    // Request Exit
    compat_request_exit();
}
void init_compat() {
    // Install Exit Handler
    signal(SIGINT, exit_handler);
    signal(SIGTERM, exit_handler);
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
