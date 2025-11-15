#include "internal.h"

// Init
CustomTesselator::CustomTesselator(const bool create_buffers) {
    if (create_buffers) {
        buffer_count = 128;
        buffers = new GLuint[buffer_count];
    } else {
        buffers = nullptr;
        buffer_count = 0;
    }
    next_buffer_index = 0;
    are_vertices_flat = false;
    enable_real_quads = false;
    offset.x = 0;
    offset.y = 0;
    offset.z = 0;
    reset_scale();
    clear();
}
void CustomTesselator::clear() {
    vertices.clear();
    vertices_flat.clear();
    active = false;
    void_begin_end = false;
    uv.reset();
    no_color = false;
    color.reset();
    normal.reset();
    quad_to_triangle_tracker = 0;
}
void CustomTesselator::reset_scale() {
    scale_x = 1;
    scale_y = 1;
}
static void Tesselator_clear_injection(MCPI_UNUSED Tesselator *self) {
    advanced_tesselator_get().clear();
}
static void Tesselator_init_injection(MCPI_UNUSED Tesselator *self) {
    const CustomTesselator &t = advanced_tesselator_get();
    if (t.buffer_count > 0) {
        media_glGenBuffers(t.buffer_count, t.buffers);
    }
}
CustomTesselator::~CustomTesselator() {
    if (buffer_count > 0) {
        media_glDeleteBuffers(buffer_count, buffers);
        delete[] buffers;
    }
}

// Begin
static void Tesselator_begin_injection(MCPI_UNUSED Tesselator *self, const int mode) {
    CustomTesselator &t = advanced_tesselator_get();
    if (t.void_begin_end) {
        return;
    } else if (t.active) {
        ERR("Already Tessellating");
    }
    t.clear();
    t.active = true;
    t.mode = mode;
}
static void Tesselator_voidBeginAndEndCalls_injection(Tesselator *self, const bool x) {
    CustomTesselator &t = advanced_tesselator_get();
    t.void_begin_end = x;
    if (t.buffer_count > 0) {
        // On Main-Thread
        self->void_begin_end = x;
    }
}

// Patch
void _tesselator_init_state() {
    replace(Tesselator_init);
    replace(Tesselator_clear);
    replace(Tesselator_begin);
    replace(Tesselator_voidBeginAndEndCalls);
}