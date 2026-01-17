#include <cstdint>

#include <SDL/SDL.h>

#include <media-layer/core.h>
#include <media-layer/audio.h>

#include "common/common.h"
#include "opengl/state.h"

// SDL Functions

CALL(0, media_SDL_Init, int, (const uint32_t flags))
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

CALL(5, media_SDL_WM_GrabInput, SDL_GrabMode, (const SDL_GrabMode mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return (SDL_GrabMode) trampoline(false, mode);
#else
    return func(args.next<SDL_GrabMode>());
#endif
}

CALL(6, media_SDL_ShowCursor, int, (const int32_t toggle))
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

CALL(59, media_audio_update, void, (const float volume, const float x, const float y, const float z, const float yaw))
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

CALL(60, media_audio_play, void, (const char *source, const char *name, const float x, const float y, const float z, const float pitch, const float volume, const int is_ui))
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

CALL(62, media_set_interactable, void, (const bool is_interactable))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, is_interactable);
#else
    func(args.next<bool>());
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

CALL(64, media_set_raw_mouse_motion_enabled, void, (const bool enabled))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, enabled);
#else
    func(args.next<bool>());
    return 0;
#endif
}

CALL(76, media_begin_offscreen_render, void, (const unsigned int texture))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, gl_state.bound_texture, texture);
#else
    media_glBindTexture(GL_TEXTURE_2D, args.next<GLuint>());
    const unsigned int texture = args.next<uint32_t>();
    func(texture);
    return 0;
#endif
}

CALL(77, media_end_offscreen_render, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true);
#else
    func();
    return 0;
#endif
}

CALL(78, media_download_into_texture, void, (const unsigned int texture, const char *url))
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

CALL(80, media_open, void, (const char *path, const bool is_url))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, copy_array(path), is_url);
#else
    const char *path = args.next_arr<char>();
    const bool is_url = args.next<bool>();
    func(path, is_url);
    return 0;
#endif
}