#pragma once

#include <GLES/gl.h>

// Track GL State
struct gl_array_details_t {
    bool dirty = false;
    bool enabled = false;
    GLint size = 0;
    GLenum type = 0;
    GLsizei stride = 0;
    uint32_t pointer = 0;
    void update(bool enabled_, GLint size_, GLenum type_, GLsizei stride_, uint32_t pointer_);
};
struct gl_state_t {
    GLuint bound_array_buffer = 0;
    GLuint bound_texture = 0;
    struct array_details_t {
        gl_array_details_t media_glVertexPointer;
        gl_array_details_t media_glColorPointer;
        gl_array_details_t media_glTexCoordPointer;
        gl_array_details_t media_glNormalPointer;
        void set_all_dirty(bool x);
    } array_details;

    // Track When To Send Bound Buffer/Texture Objects
    struct {
        bool state = false;
        bool should_include_texture = false;
    } in_display_list;

    // Get Array Details
    gl_array_details_t &get_array_details(GLenum array);
    [[nodiscard]] const gl_array_details_t &get_array_details_const(GLenum array) const;

#ifndef MEDIA_LAYER_TRAMPOLINE_GUEST
    // Send Array State To Driver
    void send_array_to_driver(GLenum array) const;
    // Send State To Driver
    void send_to_driver() const;
#endif
};

// Global State
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
MCPI_INTERNAL extern gl_state_t gl_state;
#endif