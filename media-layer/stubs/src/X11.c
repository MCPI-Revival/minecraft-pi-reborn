#include <X11/Xlib.h>

#include <libreborn/libreborn.h>

// Raw X11 Is Replaced With GLFW

int XTranslateCoordinates(__attribute__((unused)) void *display, __attribute__((unused)) XID src_w, __attribute__((unused)) XID dest_w, __attribute__((unused)) int src_x, __attribute__((unused)) int src_y, __attribute__((unused)) int *dest_x_return, __attribute__((unused)) int *dest_y_return, __attribute__((unused)) XID *child_return) {
    IMPOSSIBLE();
}
int XGetWindowAttributes(__attribute__((unused)) void *display, __attribute__((unused)) XID w, __attribute__((unused)) XWindowAttributes *window_attributes_return) {
    IMPOSSIBLE();
}
