#include <symbols/Tesselator.h>

#include "internal.h"

// Init
CustomTesselator::CustomTesselator(const bool create_buffers) {
    has_buffer = create_buffers;
    are_vertices_flat = false;
    offset.x = 0;
    offset.y = 0;
    offset.z = 0;
    clear(true);
}
static void Tesselator_init_injection(MCPI_UNUSED Tesselator *self) {
    CustomTesselator &t = advanced_tesselator_get();
    if (t.has_buffer) {
        media_glGenBuffers(1, &t.buffer);
        t.buffer_size = 0;
    }
}
CustomTesselator::~CustomTesselator() {
    if (has_buffer) {
        media_glDeleteBuffers(1, &buffer);
    }
}

// (Partially) Reset State
void CustomTesselator::clear(const bool full) {
    vertices.clear();
    vertices_flat.clear();
    voidBeginAndEndCalls(false);
    if (full) {
        uv.reset();
        no_color = false;
        color.reset();
        normal.reset();
        active = false;
        reset_scale();
    }
}
void CustomTesselator::reset_scale() {
    scale_x = 1;
    scale_y = 1;
}
static void Tesselator_clear_injection(MCPI_UNUSED Tesselator *self) {
    advanced_tesselator_get().clear(false);
}

// Begin
static void Tesselator_begin_injection(MCPI_UNUSED Tesselator *self, const int mode) {
    CustomTesselator &t = advanced_tesselator_get();
    if (t.void_begin_end) {
        return;
    } else if (t.active) {
        ERR("Already Tessellating");
    }
    t.clear(true);
    t.active = true;
    t.mode = mode;
}
void CustomTesselator::voidBeginAndEndCalls(const bool x) {
    void_begin_end = x;
    if (has_buffer) {
        // On Main-Thread
        Tesselator::instance.void_begin_end = x;
    }
}
static void Tesselator_voidBeginAndEndCalls_injection(MCPI_UNUSED Tesselator *self, const bool x) {
    advanced_tesselator_get().voidBeginAndEndCalls(x);
}

// Patch
void _tesselator_init_state() {
    replace(Tesselator_init);
    replace(Tesselator_clear);
    replace(Tesselator_begin);
    replace(Tesselator_voidBeginAndEndCalls);
}