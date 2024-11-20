#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Default Window Size
#define DEFAULT_WIDTH 840
#define DEFAULT_HEIGHT 480

// SDL User Events
#define USER_EVENT_CHARACTER 0 // data1 = 8-Bit Character
#define USER_EVENT_REAL_KEY 1 // data1 = SDL_RELEASED/PRESSED, data2 = GLFW Key Code

void media_toggle_fullscreen();
void media_swap_buffers();
void media_cleanup();
void media_get_framebuffer_size(int *width, int *height);
void media_set_interactable(int is_interactable);
void media_disable_vsync();
void media_set_raw_mouse_motion_enabled(int enabled);
int media_has_extension(const char *name);
void media_begin_offscreen_render(int width, int height);
void media_end_offscreen_render();

#ifdef __cplusplus
}
#endif
