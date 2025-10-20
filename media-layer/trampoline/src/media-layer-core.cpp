#include <cstdint>

#include <SDL/SDL.h>

#include <media-layer/core.h>
#include <media-layer/audio.h>

#include "common/common.h"

// SDL Functions

CALL(0, media_SDL_Init, int, (uint32_t flags))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return trampoline(false, flags);
#else
    const uint32_t flags = args.next<uint32_t>();
    return func(flags);
#endif
}

CALL(1, media_SDL_PollEvent, int, (SDL_Event *event))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return trampoline(false, uint32_t(event));
#else
    SDL_Event event;
    const int ret = func(&event);
    writer(args.next<uint32_t>(), &event, sizeof(SDL_Event));
    return ret;
#endif
}

CALL(2, media_SDL_PushEvent, int, (const SDL_Event *event))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return trampoline(false, *event);
#else
    const SDL_Event &event = args.next<SDL_Event>();
    return func(&event);
#endif
}

CALL(3, media_SDL_WM_SetCaption, void, (const char *title, const char *icon))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, copy_array(title), copy_array(icon));
#else
    const char *title = args.next_arr<char>();
    const char *icon = args.next_arr<char>();
    func(title, icon);
    return 0;
#endif
}

CALL(4, media_toggle_fullscreen, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true);
#else
    func();
    return 0;
#endif
}

CALL(5, media_SDL_WM_GrabInput, SDL_GrabMode, (SDL_GrabMode mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return (SDL_GrabMode) trampoline(false, mode);
#else
    return func(args.next<SDL_GrabMode>());
#endif
}

CALL(6, media_SDL_ShowCursor, int, (int32_t toggle))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return trampoline(false, toggle);
#else
    return func(args.next<int32_t>());
#endif
}

CALL(8, media_swap_buffers, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true);
#else
    func();
    return 0;
#endif
}

CALL(9, media_cleanup, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(false);
#else
    func();
    return 0;
#endif
}

CALL(10, media_get_framebuffer_size, void, (int *width, int *height))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(false, uint32_t(width), uint32_t(height));
#else
    int width;
    int height;
    func(&width, &height);
    writer(args.next<uint32_t>(), &width, sizeof(int));
    writer(args.next<uint32_t>(), &height, sizeof(int));
    return 0;
#endif
}

CALL(59, media_audio_update, void, (float volume, float x, float y, float z, float yaw))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, volume, x, y, z, yaw);
#else
    const float volume = args.next<float>();
    const float x = args.next<float>();
    const float y = args.next<float>();
    const float z = args.next<float>();
    const float yaw = args.next<float>();
    func(volume, x, y, z, yaw);
    return 0;
#endif
}

CALL(60, media_audio_play, void, (const char *source, const char *name, float x, float y, float z, float pitch, float volume, int is_ui))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, copy_array(source), copy_array(name), x, y, z, pitch, volume, is_ui);
#else
    const char *source = args.next_arr<char>();
    const char *name = args.next_arr<char>();
    const float x = args.next<float>();
    const float y = args.next<float>();
    const float z = args.next<float>();
    const float pitch = args.next<float>();
    const float volume = args.next<float>();
    const int is_ui = args.next<int>();
    func(source, name, x, y, z, pitch, volume, is_ui);
    return 0;
#endif
}

CALL(62, media_set_interactable, void, (int is_interactable))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, is_interactable);
#else
    func(args.next<int>());
    return 0;
#endif
}

CALL(63, media_disable_vsync, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true);
#else
    func();
    return 0;
#endif
}

CALL(64, media_set_raw_mouse_motion_enabled, void, (int enabled))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, enabled);
#else
    func(args.next<int>());
    return 0;
#endif
}

CALL(71, media_has_extension, int, (const char *name))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return trampoline(false, copy_array(name));
#else
    return func(args.next_arr<char>());
#endif
}

CALL(76, media_begin_offscreen_render, void, (const int width, const int height))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, width, height);
    _media_backup_gl_state();
#else
    const int width = args.next<int32_t>();
    const int height = args.next<int32_t>();
    func(width, height);
    return 0;
#endif
}

CALL(77, media_end_offscreen_render, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true);
    _media_restore_gl_state();
#else
    func();
    return 0;
#endif
}

CALL(78, media_download_into_texture, void, (unsigned int texture, const char *url))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, texture, copy_array(url));
#else
    const unsigned int texture = args.next<uint32_t>();
    const char *url = args.next_arr<char>();
    func(texture, url);
    return 0;
#endif
}

CALL(79, media_apply_downloaded_textures, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true);
#else
    func();
    return 0;
#endif
}