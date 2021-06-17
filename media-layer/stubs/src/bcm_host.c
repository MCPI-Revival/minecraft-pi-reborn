#include <stdint.h>

void bcm_host_init(void) {
}

void bcm_host_deinit(void) {
}

int32_t graphics_get_display_size(__attribute__((unused)) const uint16_t display_number, __attribute__((unused)) uint32_t *width, __attribute__((unused)) uint32_t *height) {
    return -1;
}

uint32_t vc_dispmanx_display_open(__attribute__((unused)) uint32_t device) {
    return 0;
}

uint32_t vc_dispmanx_element_add(__attribute__((unused)) uint32_t update, __attribute__((unused)) uint32_t display, __attribute__((unused)) int32_t layer, __attribute__((unused)) const void *dest_rect, __attribute__((unused)) uint32_t src, __attribute__((unused)) const void *src_rect, __attribute__((unused)) uint32_t protection, __attribute__((unused)) void *alpha, __attribute__((unused)) void *clamp, __attribute__((unused)) uint32_t transform) {
    return 0;
}

uint32_t vc_dispmanx_update_start(__attribute__((unused)) int32_t priority) {
    return 0;
}

int vc_dispmanx_update_submit_sync(__attribute__((unused)) uint32_t update) {
    return 0;
}
