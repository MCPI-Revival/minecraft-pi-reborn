#include <stdint.h>

#include <SDL/SDL.h>

#include <libreborn/libreborn.h>
#include <media-layer/core.h>
#include <media-layer/audio.h>
#include <media-layer/internal.h>

#include "common/common.h"

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
        safe_read((void *) event, sizeof (SDL_Event));
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
        safe_write((void *) &event, sizeof (SDL_Event));
    }
#endif
}

CALL(2, SDL_PushEvent, int, (SDL_Event *event)) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();

    // Arguments
    safe_write((void *) event, sizeof (SDL_Event));

    // Get Return Value
    int32_t ret = (int32_t) read_int();

    // Release Proxy
    end_proxy_call();

    // Return Value
    return ret;
#else
    SDL_Event event;
    safe_read((void *) &event, sizeof (SDL_Event));
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

CALL(6, SDL_ShowCursor, int, (int32_t toggle)) {
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

CALL(7, media_take_screenshot, void, (char *home)) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();

    // Arguments
    write_string(home);

    // Release Proxy
    end_proxy_call();
#else
    char *home = read_string();
    // Run
    media_take_screenshot(home);
    // Free
    free(home);
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

CALL(59, media_audio_update, void, (float volume, float x, float y, float z, float yaw)) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();

    // Arguments
    write_float(volume);
    write_float(x);
    write_float(y);
    write_float(z);
    write_float(yaw);

    // Release Proxy
    end_proxy_call();
#else
    float volume = read_float();
    float x = read_float();
    float y = read_float();
    float z = read_float();
    float yaw = read_float();
    // Run
    media_audio_update(volume, x, y, z, yaw);
#endif
}

CALL(60, media_audio_play, void, (const char *source, const char *name, float x, float y, float z, float pitch, float volume, int is_ui)) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();

    // Arguments
    write_string(source);
    write_string(name);
    write_float(x);
    write_float(y);
    write_float(z);
    write_float(pitch);
    write_float(volume);
    write_int(is_ui);

    // Release Proxy
    end_proxy_call();
#else
    char *source = read_string();
    char *name = read_string();
    float x = read_float();
    float y = read_float();
    float z = read_float();
    float pitch = read_float();
    float volume = read_float();
    int is_ui = read_int();
    // Run
    media_audio_play(source, name, x, y, z, pitch, volume, is_ui);
    // Free
    free(source);
    free(name);
#endif
}

CALL(62, media_set_interactable, void, (int is_interactable)) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();

    // Arguments
    write_int(is_interactable);

    // Release Proxy
    end_proxy_call();
#else
    int is_interactable = read_int();
    // Run
    media_set_interactable(is_interactable);
#endif
}

CALL(63, media_disable_vsync, void, ()) {
#if defined(MEDIA_LAYER_PROXY_SERVER)
    // Lock Proxy
    start_proxy_call();
    // Release Proxy
    end_proxy_call();
#else
    // Run
    media_disable_vsync();
#endif
}
