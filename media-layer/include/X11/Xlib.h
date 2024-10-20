#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long XID;

typedef struct {
    int x, y;
    int width, height;
    int border_width;
    int depth;
    void *visual;
    XID root;
    int clazz;
    int bit_gravity;
    int win_gravity;
    int backing_store;
    unsigned long backing_planes;
    unsigned long backing_pixel;
    int save_under;
    XID colormap;
    int map_installed;
    int map_state;
    long all_event_masks;
    long your_event_mask;
    long do_not_propagate_mask;
    int override_redirect;
    void *screen;
} XWindowAttributes;

#ifdef __cplusplus
}
#endif
