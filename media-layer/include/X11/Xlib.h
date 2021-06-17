#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

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

int XTranslateCoordinates(void *display, XID src_w, XID dest_w, int src_x, int src_y, int *dest_x_return, int *dest_y_return, XID *child_return);
int XGetWindowAttributes(void *display, XID w, XWindowAttributes *window_attributes_return);

#ifdef __cplusplus
}
#endif
