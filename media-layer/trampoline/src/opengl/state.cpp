#include "state.h"

#include <libreborn/log.h>

// Track GL State
void gl_array_details_t::update(const bool enabled_, const GLint size_, const GLenum type_, const GLsizei stride_,
    const uint32_t pointer_) {
    if (enabled != enabled_ || size != size_ || type != type_ || stride != stride_ || pointer != pointer_) {
        // Array Details Have Changed!
        enabled = enabled_;
        size = size_;
        type = type_;
        stride = stride_;
        pointer = pointer_;
        dirty = true;
    }
}
void gl_state_t::array_details_t::set_all_dirty(const bool x) {
    media_glVertexPointer.dirty = x;
    media_glColorPointer.dirty = x;
    media_glTexCoordPointer.dirty = x;
    media_glNormalPointer.dirty = x;
}

// Get OpenGL Array Details
gl_array_details_t &gl_state_t::get_array_details(const GLenum array) {
    switch (array) {
        case GL_VERTEX_ARRAY: return array_details.media_glVertexPointer;
        case GL_COLOR_ARRAY: return array_details.media_glColorPointer;
        case GL_TEXTURE_COORD_ARRAY: return array_details.media_glTexCoordPointer;
        case GL_NORMAL_ARRAY: return array_details.media_glNormalPointer;
        default: ERR("Unsupported Array Type: %i", array);
    }
}
const gl_array_details_t &gl_state_t::get_array_details_const(const GLenum array) const {
    return const_cast<gl_state_t *>(this)->get_array_details(array);
}

// Send Array State To Driver
#ifndef MEDIA_LAYER_TRAMPOLINE_GUEST
void gl_state_t::send_array_to_driver(const GLenum array) const {
    const gl_array_details_t &state = get_array_details_const(array);
    if (!in_display_list.state && !state.dirty) {
        return;
    }
    if (state.enabled) {
        media_glEnableClientState(array);
        switch (array) {
            case GL_VERTEX_ARRAY: media_glVertexPointer(state.size, state.type, state.stride, (void *) (uintptr_t) state.pointer); break;
            case GL_COLOR_ARRAY: media_glColorPointer(state.size, state.type, state.stride, (void *) (uintptr_t) state.pointer); break;
            case GL_TEXTURE_COORD_ARRAY: media_glTexCoordPointer(state.size, state.type, state.stride, (void *) (uintptr_t) state.pointer); break;
            case GL_NORMAL_ARRAY: media_glNormalPointer(state.type, state.stride, (void *) (uintptr_t) state.pointer); break;
            default: IMPOSSIBLE();
        }
    } else {
        media_glDisableClientState(array);
    }
}
void gl_state_t::send_to_driver() const {
    if (!in_display_list.state || in_display_list.should_include_texture) {
        media_glBindTexture(GL_TEXTURE_2D, bound_texture);
    }
    media_glBindBuffer(GL_ARRAY_BUFFER, bound_array_buffer);
    send_array_to_driver(GL_VERTEX_ARRAY);
    send_array_to_driver(GL_COLOR_ARRAY);
    send_array_to_driver(GL_TEXTURE_COORD_ARRAY);
    send_array_to_driver(GL_NORMAL_ARRAY);
}
#endif

// Global State
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
gl_state_t gl_state;
#endif
