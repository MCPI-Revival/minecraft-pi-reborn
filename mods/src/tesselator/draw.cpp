#include "internal.h"

// Draw To Buffer
static GLuint get_next_buffer(CustomTesselator &t) {
    if (t.buffer_count == 0) {
        ERR("Rendering Must Happen On Main Thread");
    }
    t.next_buffer_index++;
    t.next_buffer_index %= t.buffer_count;
    const GLuint out = t.buffers[t.next_buffer_index];
    return out;
}
#define NOT_TESSELLATING() ERR("Not Tessellating")
TEMPLATE
static RenderChunk Tesselator_end_injection_impl(CustomTesselator &t, const bool use_given_buffer, const int buffer) {
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
        out.buffer = use_given_buffer ? buffer : get_next_buffer(t);
        media_glBindBuffer(GL_ARRAY_BUFFER, out.buffer);
        const uint vertex_size = sizeof(Vertex);
        media_glBufferData(GL_ARRAY_BUFFER, out.vertices * vertex_size, vertices.data, use_given_buffer ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
    }

    // Finish
    t.clear();
    return out;
}
static RenderChunk Tesselator_end_injection(MCPI_UNUSED Tesselator *self, const bool use_given_buffer, const int buffer) {
    CustomTesselator &t = advanced_tesselator_get();
    if (t.are_vertices_flat) {
        return Tesselator_end_injection_impl<CustomVertexFlat, &CustomTesselator::vertices_flat>(t, use_given_buffer, buffer);
    } else {
        return Tesselator_end_injection_impl<CustomVertexShaded, &CustomTesselator::vertices>(t, use_given_buffer, buffer);
    }
}

// Setup/Cleanup State For Drawing
static void begin_draw(const GLsizei vertex_size, const bool has_texture, const bool has_color, const bool has_normal) {
    if (has_texture) {
        media_glTexCoordPointer(2, GL_FLOAT, vertex_size, (void *) offsetof(CustomVertexFlat, uv));
        media_glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if (has_color) {
        media_glColorPointer(4, GL_UNSIGNED_BYTE, vertex_size, (void *) offsetof(CustomVertexFlat, color));
        media_glEnableClientState(GL_COLOR_ARRAY);
    }
    if (has_normal) {
        media_glNormalPointer(GL_BYTE, vertex_size, (void *) offsetof(CustomVertexShaded, normal));
        media_glEnableClientState(GL_NORMAL_ARRAY);
    }
    media_glVertexPointer(3, GL_FLOAT, vertex_size, (void *) offsetof(CustomVertexFlat, pos));
    media_glEnableClientState(GL_VERTEX_ARRAY);
}
static void end_draw() {
    media_glDisableClientState(GL_VERTEX_ARRAY);
    media_glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    media_glDisableClientState(GL_COLOR_ARRAY);
    media_glDisableClientState(GL_NORMAL_ARRAY);
}

// Draw To Screen
TEMPLATE
static void Tesselator_draw_injection_impl(CustomTesselator &t) {
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
        const GLuint buffer = get_next_buffer(t);
        media_glBindBuffer(GL_ARRAY_BUFFER, buffer);
        media_glBufferData(GL_ARRAY_BUFFER, vertex_count * vertex_size, vertices.data, GL_DYNAMIC_DRAW);

        // Draw
        begin_draw(vertex_size, t.uv.has_value(), t.color.has_value(), t.normal.has_value());
        GLenum mode = t.mode;
        if (!t.enable_real_quads && mode == GL_QUADS) {
            mode = GL_TRIANGLES;
        }
        media_glDrawArrays(mode, 0, vertex_count);
        end_draw();
    }

    // Finish
    t.clear();
}
static void Tesselator_draw_injection(MCPI_UNUSED Tesselator *self) {
    CustomTesselator &t = advanced_tesselator_get();
    if (t.are_vertices_flat) {
        Tesselator_draw_injection_impl<CustomVertexFlat, &CustomTesselator::vertices_flat>(t);
    } else {
        Tesselator_draw_injection_impl<CustomVertexShaded, &CustomTesselator::vertices>(t);
    }
}

// Draw Buffer
static void Common_drawArrayVT_injection(const int buffer, const int vertices, int vertex_size, const uint mode) {
    // Check
    if (advanced_tesselator_get().are_vertices_flat) {
        IMPOSSIBLE();
    }

    // Draw
    media_glBindBuffer(GL_ARRAY_BUFFER, buffer);
    vertex_size = sizeof(CustomVertexShaded);
    begin_draw(vertex_size, true, false, true);
    media_glDrawArrays(mode, 0, vertices);
    end_draw();
}

// Patch
void _tesselator_init_draw() {
    replace(Tesselator_end);
    replace(Tesselator_draw);
    replace(Common_drawArrayVT);
}