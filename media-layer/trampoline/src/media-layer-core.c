#include <stdint.h>

#include <SDL/SDL.h>

#include <libreborn/libreborn.h>
#include <media-layer/core.h>
#include <media-layer/audio.h>

#include "common/common.h"

// SDL Functions

CALL(0, SDL_Init, int, (uint32_t flags))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return trampoline(flags);
#else
    uint32_t flags = next_int();
    // Run
    ret(func(flags));
#endif
}

CALL(1, SDL_PollEvent, int, (SDL_Event *event))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return trampoline((uint32_t) event);
#else
    SDL_Event *event = next_ptr();
    // Run
    ret(func(event));
#endif
}

CALL(2, SDL_PushEvent, int, (SDL_Event *event))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return trampoline((uint32_t) event);
#else
    SDL_Event *event = next_ptr();
    // Run
    ret(func(event));
#endif
}

CALL(3, SDL_WM_SetCaption, void, (const char *title, const char *icon))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline((uint32_t) title, (uint32_t) icon);
#else
    char *title = next_ptr();
    char *icon = next_ptr();
    // Run
    func(title, icon);
#endif
}

CALL(4, media_toggle_fullscreen, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline();
#else
    // Run
    func();
#endif
}

CALL(5, SDL_WM_GrabInput, SDL_GrabMode, (SDL_GrabMode mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return trampoline(mode);
#else
    SDL_GrabMode mode = next_int();
    // Run
    ret(func(mode));
#endif
}

CALL(6, SDL_ShowCursor, int, (int32_t toggle))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return trampoline(toggle);
#else
    int mode = next_int();
    // Run
    ret(func(mode));
#endif
}

CALL(8, media_swap_buffers, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline();
#else
    // Run
    func();
#endif
}

CALL(9, media_cleanup, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline();
#else
    // Run
    func();
#endif
}

CALL(10, media_get_framebuffer_size, void, (int *width, int *height))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline((uint32_t) width, (uint32_t) height);
#else
    int *width = next_ptr();
    int *height = next_ptr();
    // Run
    func(width, height);
#endif
}

CALL(59, media_audio_update, void, (float volume, float x, float y, float z, float yaw))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(pun_to(uint32_t, volume), pun_to(uint32_t, x), pun_to(uint32_t, y), pun_to(uint32_t, z), pun_to(uint32_t, yaw));
#else
    float volume = next_float();
    float x = next_float();
    float y = next_float();
    float z = next_float();
    float yaw = next_float();
    // Run
    func(volume, x, y, z, yaw);
#endif
}

CALL(60, media_audio_play, void, (const char *source, const char *name, float x, float y, float z, float pitch, float volume, int is_ui))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline((uint32_t) source, (uint32_t) name, pun_to(uint32_t, x), pun_to(uint32_t, y), pun_to(uint32_t, z), pun_to(uint32_t, pitch), pun_to(uint32_t, volume), is_ui);
#else
    char *source = next_ptr();
    char *name = next_ptr();
    float x = next_float();
    float y = next_float();
    float z = next_float();
    float pitch = next_float();
    float volume = next_float();
    int is_ui = next_int();
    // Run
    func(source, name, x, y, z, pitch, volume, is_ui);
#endif
}

CALL(62, media_set_interactable, void, (int is_interactable))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(is_interactable);
#else
    int is_interactable = next_int();
    // Run
    func(is_interactable);
#endif
}

CALL(63, media_disable_vsync, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline();
#else
    // Run
    func();
#endif
}

CALL(64, media_set_raw_mouse_motion_enabled, void, (int enabled))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(enabled);
#else
    int enabled = next_int();
    // Run
    func(enabled);
#endif
}

CALL(66, media_force_egl, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline();
#else
    // Run
    func();
#endif
}

CALL(68, media_ensure_loaded, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline();
#else
    // Run
    func();
#endif
}
