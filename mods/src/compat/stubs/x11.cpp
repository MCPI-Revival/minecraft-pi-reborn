#include <X11/Xlib.h>

#include <libreborn/patch.h>
#include <media-layer/core.h>
#include "../internal.h"

// Functions That Have Their Return Values Used
static int XTranslateCoordinates_injection(MCPI_UNUSED void *display, MCPI_UNUSED XID src_w, MCPI_UNUSED XID dest_w, int src_x, int src_y, int *dest_x_return, int *dest_y_return, MCPI_UNUSED XID *child_return) {
    // Use MCPI Replacement Function
    *dest_x_return = src_x;
    *dest_y_return = src_y;
    return 1;
}
static int XGetWindowAttributes_injection(MCPI_UNUSED void *display, MCPI_UNUSED XID w, XWindowAttributes *window_attributes_return) {
    // Use MCPI Replacement Function
    XWindowAttributes attributes = {};
    attributes.x = 0;
    attributes.y = 0;
    media_get_framebuffer_size(&attributes.width, &attributes.height);
    *window_attributes_return = attributes;
    return 1;
}

// Patch X11 Calls
void _patch_x11_calls() {
    // Disable X11 Calls
    overwrite_call_manual((void *) 0x132a4, (void *) XGetWindowAttributes_injection); // XGetWindowAttributes
    overwrite_call_manual((void *) 0x132d4, (void *) XTranslateCoordinates_injection); // XTranslateCoordinates
}
