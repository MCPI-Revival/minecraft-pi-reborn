#include <symbols/Tesselator.h>
#include <symbols/Common.h>

#include "internal.h"

// Draw To Buffer
#define NOT_TESSELLATING() ERR("Not Tessellating")
TEMPLATE
__attribute__((hot, always_inline)) static inline RenderChunk Tesselator_end_injection_impl(CustomTesselator &t, const bool use_given_buffer, const int buffer) {
    // Check
    if (!t.active) {
        NOT_TESSELLATING();
    }
    RenderChunk out;
    out.constructor();
    if (t.void_begin_end) {
        return out;
    }

    // Render
    const VertexArray<Vertex> &vertices = t.*VertexPtr;
    out.vertices = vertices.size;
    if (out.vertices > 0) {
        if (!use_given_buffer) {
            WARN("You Should Not Do This");
        }
        out.buffer = use_given_buffer ? buffer : t.buffer;
        media_glBindBuffer(GL_ARRAY_BUFFER, out.buffer);
        const uint vertex_size = sizeof(Vertex);
        media_glBufferData(GL_ARRAY_BUFFER, out.vertices * vertex_size, vertices.data, use_given_buffer ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
    }

    // Finish
    t.clear();
    return out;
}
__attribute__((hot)) static RenderChunk Tesselator_end_injection(MCPI_UNUSED Tesselator *self, const bool use_given_buffer, const int buffer) {
    CustomTesselator &t = advanced_tesselator_get();
    if (t.are_vertices_flat) {
        return Tesselator_end_injection_impl<CustomVertexFlat, &CustomTesselator::vertices_flat>(t, use_given_buffer, buffer);
    } else {
        return Tesselator_end_injection_impl<CustomVertexShaded, &CustomTesselator::vertices>(t, use_given_buffer, buffer);
    }
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
__attribute__((hot, always_inline)) static inline void Tesselator_draw_injection_impl(CustomTesselator &t) {
    // Check
    if (!t.active) {
        NOT_TESSELLATING();
    }
    if (t.void_begin_end) {
        return;
    }

    // Render
    const VertexArray<Vertex> &vertices = t.*VertexPtr;
    const int vertex_count = vertices.size;
    if (vertex_count > 0) {
        // Upload
        constexpr uint vertex_size = sizeof(Vertex);
        const GLuint buffer = t.buffer;
        media_glBindBuffer(GL_ARRAY_BUFFER, buffer);
        GLsizeiptr &buffer_size = t.buffer_size;
        const GLsizeiptr data_size = vertex_count * vertex_size;
        if (data_size > buffer_size) {
            media_glBufferData(GL_ARRAY_BUFFER, data_size, vertices.data, GL_DYNAMIC_DRAW);
            buffer_size = data_size;
        } else {
            media_glBufferSubData(GL_ARRAY_BUFFER, 0, data_size, vertices.data);
        }

        // Draw
        begin_draw(vertex_size, t.uv.has_value(), t.color.has_value(), t.normal.has_value());
        media_glDrawArrays(t.mode, 0, vertex_count);
    }

    // Finish
    t.clear();
}
__attribute__((hot)) static void Tesselator_draw_injection(MCPI_UNUSED Tesselator *self) {
    CustomTesselator &t = advanced_tesselator_get();
    if (t.are_vertices_flat) {
        Tesselator_draw_injection_impl<CustomVertexFlat, &CustomTesselator::vertices_flat>(t);
    } else {
        Tesselator_draw_injection_impl<CustomVertexShaded, &CustomTesselator::vertices>(t);
    }
}

// Draw Buffer
__attribute__((hot)) static void Common_drawArrayVT_injection(const int buffer, int vertices, int vertex_size, uint mode) {
    // Check
    if (advanced_tesselator_get().are_vertices_flat) {
        IMPOSSIBLE();
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

// Patch
void _tesselator_init_draw() {
    replace(Tesselator_end);
    replace(Tesselator_draw);
    replace(Common_drawArrayVT);
}