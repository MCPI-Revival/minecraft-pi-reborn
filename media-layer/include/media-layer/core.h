#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Default Window Size
#define DEFAULT_WIDTH 840
#define DEFAULT_HEIGHT 480

void media_ensure_loaded();

void media_take_screenshot(char *home);
void media_toggle_fullscreen();
void media_swap_buffers();
void media_cleanup();
void media_get_framebuffer_size(int *width, int *height);
void media_set_interactable(int is_interactable);
void media_disable_vsync();
void media_set_raw_mouse_motion_enabled(int enabled);

#ifdef __cplusplus
}
#endif
