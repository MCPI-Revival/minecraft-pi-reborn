#include <symbols/Tesselator.h>
#include <symbols/Common.h>

#include "internal.h"

// Draw To Buffer
__attribute__((hot)) static RenderChunk Tesselator_end_injection(MCPI_UNUSED Tesselator *self, const bool use_given_buffer, const int buffer) {
    if (!use_given_buffer) {
        WARN("You Should Not Do This");
    }
    RenderChunk out = {};
    out.constructor();
    CustomTesselator &t = advanced_tesselator_get();
    out.vertices = t.draw(false, use_given_buffer ? std::optional(buffer) : std::nullopt);
    out.buffer = use_given_buffer ? buffer : t.buffer;
    return out;
}

// Setup/Cleanup State For Drawing
__attribute__((hot, always_inline)) static inline void begin_draw(const GLsizei vertex_size, const bool has_texture, const bool has_color, const bool has_normal) {
    if (has_texture) {
        media_glTexCoordPointer(2, GL_FLOAT, vertex_size, (void *) offsetof(CustomVertexFlat, uv));
        media_glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    } else {
        media_glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if (has_color) {
        media_glColorPointer(4, GL_UNSIGNED_BYTE, vertex_size, (void *) offsetof(CustomVertexFlat, color));
        media_glEnableClientState(GL_COLOR_ARRAY);
    } else {
        media_glDisableClientState(GL_COLOR_ARRAY);
    }
    if (has_normal) {
        media_glNormalPointer(GL_BYTE, vertex_size, (void *) offsetof(CustomVertexShaded, normal));
        media_glEnableClientState(GL_NORMAL_ARRAY);
    } else {
        media_glDisableClientState(GL_NORMAL_ARRAY);
    }
    media_glVertexPointer(3, GL_FLOAT, vertex_size, (void *) offsetof(CustomVertexFlat, pos));
    media_glEnableClientState(GL_VERTEX_ARRAY);
}

// Draw To Screen
TEMPLATE
__attribute__((hot, always_inline)) static inline int Tesselator_draw_injection_impl(CustomTesselator &t, const bool should_actually_draw, const std::optional<GLuint> custom_buffer) {
    // Check
    if (!t.active) {
        ERR("Not Tessellating");
    }
    if (t.void_begin_end) {
        return 0;
    }

    // Render
    const VertexArray<Vertex> &vertices = t.*VertexPtr;
    const int vertex_count = vertices.size;
    if (vertex_count > 0) {
        // Upload
        constexpr uint vertex_size = sizeof(Vertex);
        const GLsizeiptr data_size = vertex_count * vertex_size;
        if (!custom_buffer.has_value()) {
            // Use Built-In Buffer
            const GLuint buffer = t.buffer;
            media_glBindBuffer(GL_ARRAY_BUFFER, buffer);
            GLsizeiptr &buffer_size = t.buffer_size;
            if (data_size > buffer_size) {
                media_glBufferData(GL_ARRAY_BUFFER, data_size, vertices.data, GL_STREAM_DRAW);
                buffer_size = data_size;
            } else {
                media_glBufferSubData(GL_ARRAY_BUFFER, 0, data_size, vertices.data);
            }
        } else {
            // Use Custom Buffer
            const GLuint buffer = custom_buffer.value();
            media_glBindBuffer(GL_ARRAY_BUFFER, buffer);
            media_glBufferData(GL_ARRAY_BUFFER, data_size, vertices.data, GL_STATIC_DRAW);
        }

        // Draw
        if (should_actually_draw) {
            begin_draw(vertex_size, t.uv.has_value(), t.color.has_value(), t.normal.has_value());
            media_glDrawArrays(t.mode, 0, vertex_count);
        }
    }

    // Finish
    t.clear(true);
    return vertex_count;
}
int CustomTesselator::draw(const bool should_actually_draw, const std::optional<GLuint> custom_buffer) {
    if (are_vertices_flat) {
        return Tesselator_draw_injection_impl<CustomVertexFlat, &CustomTesselator::vertices_flat>(*this, should_actually_draw, custom_buffer);
    } else {
        return Tesselator_draw_injection_impl<CustomVertexShaded, &CustomTesselator::vertices>(*this, should_actually_draw, custom_buffer);
    }
}
__attribute__((hot)) static void Tesselator_draw_injection(MCPI_UNUSED Tesselator *self) {
    advanced_tesselator_get().draw(true, std::nullopt);
}

// Draw Buffer
__attribute__((hot)) static void Common_drawArrayVT_injection(const int buffer, const int vertices, int vertex_size, uint mode) {
    // Check
    if (advanced_tesselator_get().are_vertices_flat) {
        IMPOSSIBLE();
    } else if (vertices <= 0) {
        return;
    }

    // "Triangles" Are Really Quads
    if (mode == GL_TRIANGLES) {
        mode = GL_QUADS;
    }

    // Draw
    media_glBindBuffer(GL_ARRAY_BUFFER, buffer);
    vertex_size = sizeof(CustomVertexShaded);
    begin_draw(vertex_size, true, false, true);
    media_glDrawArrays(mode, 0, vertices);
}
__attribute__((hot)) static void RenderList_renderChunks_glDrawArrays_injection(MCPI_UNUSED const GLenum mode, const GLint first, const GLsizei count) {
    media_glDrawArrays(GL_QUADS, first, count);
}

// Patch
void _tesselator_init_draw() {
    replace(Tesselator_end);
    replace(Tesselator_draw);
    replace(Common_drawArrayVT);
    overwrite_call_manual((void *) 0x527b0, (void *) RenderList_renderChunks_glDrawArrays_injection);
}