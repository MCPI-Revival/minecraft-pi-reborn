#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Default Window Size
#define DEFAULT_WIDTH 840
#define DEFAULT_HEIGHT 480

void media_take_screenshot();
void media_toggle_fullscreen();
void media_swap_buffers();
void media_cleanup();
void media_get_framebuffer_size(int *width, int *height);

#ifdef __cplusplus
}
#endif
