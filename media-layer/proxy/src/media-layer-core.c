#include <stdint.h>

#include <SDL/SDL.h>

#include <libreborn/libreborn.h>
#include <libreborn/media-layer/core.h>
#include <libreborn/media-layer/internal.h>

#include "common/common.h"

// Read/Write SDL Events
static void write_SDL_Event(SDL_Event event) {
    // Write EVent Type
    write_int(event.type);
    // Write Event Details
    switch (event.type) {
        // Focus Event
        case SDL_ACTIVEEVENT: {
            write_int(event.active.gain);
            write_int(event.active.state);
            break;
        }
        // Key Press Events
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            write_int(event.key.state);
            write_int(event.key.keysym.scancode);
            write_int(event.key.keysym.sym);
            write_int(event.key.keysym.mod);
            write_int(event.key.keysym.unicode);
            break;
        }
        // Mouse Motion Event
        case SDL_MOUSEMOTION: {
            write_int(event.motion.state);
            write_int(event.motion.x);
            write_int(event.motion.y);
            write_int(event.motion.xrel);
            write_int(event.motion.yrel);
            break;
        }
        // Mouse Press Events
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            write_int(event.button.button);
            write_int(event.button.state);
            write_int(event.button.x);
            write_int(event.button.y);
            break;
        }
        // User-Specified Event (Repurposed As Unicode Character Event)
        case SDL_USEREVENT: {
            write_int(event.user.code);
            break;
        }
    }
}
static SDL_Event read_SDL_Event() {
    // Create Event
    SDL_Event event;
    event.type = read_int();
    // Read Event Details
    switch (event.type) {
        // Focus Event
        case SDL_ACTIVEEVENT: {
            event.active.gain = read_int();
            event.active.state = read_int();
            break;
        }
        // Key Press Events
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            event.key.state = read_int();
            event.key.keysym.scancode = read_int();
            event.key.keysym.sym = read_int();
            event.key.keysym.mod = read_int();
            event.key.keysym.unicode = read_int();
            break;
        }
        // Mouse Motion Event
        case SDL_MOUSEMOTION: {
            event.motion.state = read_int();
            event.motion.x = read_int();
            event.motion.y = read_int();
            event.motion.xrel = read_int();
            event.motion.yrel = read_int();
            break;
        }
        // Mouse Press Events
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            event.button.button = read_int();
            event.button.state = read_int();
            event.button.x = read_int();
            event.button.y = read_int();
            break;
        }
        // Quit Event
        case SDL_QUIT: {
            break;
        }
        // User-Specified Event (Repurposed As Unicode Character Event)
        case SDL_USEREVENT: {
            event.user.code = read_int();
            break;
        }
        // Unsupported Event
        default: {
            INFO("Unsupported SDL Event: %u", event.type);
        }
    }
    // Return
#pragma GCC diagnostic push
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    return event;
#pragma GCC diagnostic pop
}

// SDL Functions

CALL(0, SDL_Init, int, (uint32_t flags)) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();

    // Arguments
    write_int(flags);

    // Get Return Value
    int32_t ret = (int32_t) read_int();

    // Release Proxy
    end_proxy_call();

    // Return
    return ret;
#else
    uint32_t flags = read_int();
    // Run
    int ret = SDL_Init(flags);
    // Return Values
    write_int((uint32_t) ret);
#endif
}

CALL(1, SDL_PollEvent, int, (SDL_Event *event)) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();

    // No Arguments

    // Get Return Value
    int32_t ret = (int32_t) read_int();
    if (ret) {
        *event = read_SDL_Event();
    }

    // Release Proxy
    end_proxy_call();

    // Return Value
    return ret;
#else
    SDL_Event event;
    // Run
    int ret = (int32_t) SDL_PollEvent(&event);
    // Return Values
    write_int(ret);
    if (ret) {
        write_SDL_Event(event);
    }
#endif
}

CALL(2, SDL_PushEvent, int, (SDL_Event *event)) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();

    // Arguments
    write_SDL_Event(*event);

    // Get Return Value
    int32_t ret = (int32_t) read_int();

    // Release Proxy
    end_proxy_call();

    // Return Value
    return ret;
#else
    SDL_Event event = read_SDL_Event();
    // Run
    int ret = SDL_PushEvent(&event);
    // Return Value
    write_int((uint32_t) ret);
#endif
}

CALL(3, SDL_WM_SetCaption, void, (const char *title, const char *icon)) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();

    // Arguments
    write_string((char *) title);
    write_string((char *) icon);

    // Release Proxy
    end_proxy_call();
#else
    char *title = read_string();
    char *icon = read_string();
    // Run
    SDL_WM_SetCaption(title, icon);
    // Free
    free(title);
    free(icon);
#endif
}

CALL(4, media_toggle_fullscreen, void, ()) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();
    // Release Proxy
    end_proxy_call();
#else
    // Run
    media_toggle_fullscreen();
#endif
}

CALL(5, SDL_WM_GrabInput, SDL_GrabMode, (SDL_GrabMode mode)) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();

    // Arguments
    write_int((uint32_t) mode);

    // Get Return Value
    SDL_GrabMode ret = (SDL_GrabMode) read_int();

    // Release Proxy
    end_proxy_call();

    // Return Value
    return ret;
#else
    SDL_GrabMode mode = (SDL_GrabMode) read_int();
    // Run
    SDL_GrabMode ret = SDL_WM_GrabInput(mode);
    // Return Value
    write_int((uint32_t) ret);
#endif
}

CALL(6,SDL_ShowCursor, int, (int32_t toggle)) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();

    // Arguments
    write_int((uint32_t) toggle);

    // Get Return Value
    int32_t ret = (int32_t) read_int();

    // Release Proxy
    end_proxy_call();

    // Return Value
    return ret;
#else
    int mode = (int) read_int();
    // Run
    int ret = SDL_ShowCursor(mode);
    // Return Value
    write_int((uint32_t) ret);
#endif
}

CALL(7, media_take_screenshot, void, ()) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();
    // Release Proxy
    end_proxy_call();
#else
    // Run
    media_take_screenshot();
#endif
}

CALL(8, media_swap_buffers, void, ()) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();
    // Release Proxy
    end_proxy_call();
#else
    // Run
    media_swap_buffers();
#endif
}

// This Method May Be Called In A Situation Where The Proxy Is Disconnected
CALL(9, media_cleanup, void, ()) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Check Connection
    if (is_connection_open()) {
        // Lock Proxy
        start_proxy_call();
        // Close The Connection
        close_connection();
        // Release Proxy
        end_proxy_call();
    }
#else
    // Close The Connection
    close_connection();
    // Run
    media_cleanup();
#endif
}

CALL(10, media_get_framebuffer_size, void, (int *width, int *height)) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();

    // Get Return Values
    *width = (int) read_int();
    *height = (int) read_int();

    // Release Proxy
    end_proxy_call();
#else
    int width;
    int height;
    // Run
    media_get_framebuffer_size(&width, &height);
    // Return Values
    write_int((uint32_t) width);
    write_int((uint32_t) height);
#endif
}
