#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Internal Methods (Not Handled By Media Layer Proxy)

__attribute__((visibility("internal"))) void _media_handle_SDL_PollEvent();
__attribute__((visibility("internal"))) void _media_handle_SDL_Quit();

#ifdef __cplusplus
}
#endif
