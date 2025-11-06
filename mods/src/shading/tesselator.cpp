#include <optional>
#include <cstddef>
#include <algorithm>

#include <GLES/gl.h>

#include <libreborn/patch.h>
#include <symbols/minecraft.h>

#include <mods/display-lists/display-lists.h>

#include "internal.h"

// Structures
struct UV {
    float u;
    float v;
};
struct CustomVertexFlat {
    // This Matches Vanilla Vertices
    Vec3 pos;
    UV uv;
    GLuint color;
};
struct CustomVertexShaded {
    // This Adds Normal Information
    CustomVertexFlat base;
    GLuint normal;
};
template <typename T>
struct VertexArray {
    // Simple Array
    static constexpr int max_vertices = 524288;
    // Constructor
    VertexArray() {
        data = new T[max_vertices];
    }
    ~VertexArray() {
        delete[] data;
    }
    // Methods
    void push_back(const T &value) {
        if (size >= max_vertices) {
            IMPOSSIBLE();
        }
        data[size++] = value;
    }
    void clear() {
        size = 0;
    }
    // Properties
    T *data;
    int size = 0;
};
struct CustomTesselator {
    // Vertices
    VertexArray<CustomVertexShaded> vertices;
    VertexArray<CustomVertexFlat> vertices_flat;
    // Next Vertex Information
    std::optional<uint32_t> normal;
    int quad_to_triangle_tracker;

    // Global Instance
    static CustomTesselator instance;
};
CustomTesselator CustomTesselator::instance;

// Setup Vertex Array
static void Tesselator_clear_injection(Tesselator_clear_t original, Tesselator *self) {
    if (original) {
        original(self);
    }
    // Clear
    CustomTesselator::instance.vertices.clear();
    CustomTesselator::instance.vertices_flat.clear();
    CustomTesselator::instance.quad_to_triangle_tracker = 0;
    CustomTesselator::instance.normal.reset();
}
static void Tesselator_init_injection(Tesselator_init_t original, Tesselator *self) {
    original(self);
    Tesselator_clear_injection(nullptr, nullptr);
}

// Handle Tesselation Start
static void Tesselator_begin_injection(Tesselator_begin_t original, Tesselator *self, const int mode) {
    original(self, mode);
    if (!self->void_begin_end) {
        Tesselator_clear_injection(nullptr, nullptr);
    }
}

// Select Active Vertex Vector
static bool are_vertices_flat = false;
#define impl_template \
    template <typename Vertex, VertexArray<Vertex> CustomTesselator::*VertexPtr>

// Draw To Buffer
static GLuint get_next_buffer() {
    Tesselator::instance.next_buffer_id++;
    Tesselator::instance.next_buffer_id %= Tesselator::instance.buffer_count;
    const GLuint out = Tesselator::instance.buffers[Tesselator::instance.next_buffer_id];
    return out;
}
impl_template
static RenderChunk Tesselator_end_injection_impl(Tesselator *self, const bool use_given_buffer, const int buffer) {
    // Check
    if (!self->active) {
        IMPOSSIBLE();
    }
    RenderChunk out;
    out.constructor();
    if (self->void_begin_end) {
        return out;
    }
    // Render
    const VertexArray<Vertex> &vertices = CustomTesselator::instance.*VertexPtr;
    out.vertices = vertices.size;
    if (out.vertices > 0) {
        out.buffer = use_given_buffer ? buffer : get_next_buffer();
        media_glBindBuffer(GL_ARRAY_BUFFER, out.buffer);
        const uint vertex_size = sizeof(Vertex);
        media_glBufferData(GL_ARRAY_BUFFER, out.vertices * vertex_size, vertices.data, use_given_buffer ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
    }
    // Finish
    self->clear();
    self->active = false;
    return out;
}
static RenderChunk Tesselator_end_injection(Tesselator *self, const bool use_given_buffer, const int buffer) {
    if (are_vertices_flat) {
        return Tesselator_end_injection_impl<CustomVertexFlat, &CustomTesselator::vertices_flat>(self, use_given_buffer, buffer);
    } else {
        return Tesselator_end_injection_impl<CustomVertexShaded, &CustomTesselator::vertices>(self, use_given_buffer, buffer);
    }
}

// Enable Real GL_QUADS
static bool enable_real_quads = false;

// Draw To Screen
impl_template
static void Tesselator_draw_injection_impl(Tesselator *self) {
    // Check
    if (!self->active) {
        IMPOSSIBLE();
    }
    if (self->void_begin_end) {
        return;
    }
    // Render
    const VertexArray<Vertex> &vertices = CustomTesselator::instance.*VertexPtr;
    const int vertex_count = vertices.size;
    if (vertex_count > 0) {
        // Upload
        constexpr uint vertex_size = sizeof(Vertex);
        const GLuint buffer = get_next_buffer();
        media_glBindBuffer(GL_ARRAY_BUFFER, buffer);
        media_glBufferData(GL_ARRAY_BUFFER, vertex_count * vertex_size, vertices.data, GL_DYNAMIC_DRAW);
        // Setup State
        if (self->has_texture) {
            media_glTexCoordPointer(2, GL_FLOAT, vertex_size, (void *) offsetof(CustomVertexFlat, uv));
            media_glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        if (self->has_color) {
            media_glColorPointer(4, GL_UNSIGNED_BYTE, vertex_size, (void *) offsetof(CustomVertexFlat, color));
            media_glEnableClientState(GL_COLOR_ARRAY);
        }
        if (!are_vertices_flat && CustomTesselator::instance.normal) {
            media_glNormalPointer(GL_BYTE, vertex_size, (void *) offsetof(CustomVertexShaded, normal));
            media_glEnableClientState(GL_NORMAL_ARRAY);
        }
        media_glVertexPointer(3, GL_FLOAT, vertex_size, (void *) offsetof(CustomVertexFlat, pos));
        media_glEnableClientState(GL_VERTEX_ARRAY);
        int mode = self->mode;
        if (!enable_real_quads && mode == GL_QUADS) {
            mode = GL_TRIANGLES;
        }
        // Draw
        media_glDrawArrays(mode, 0, vertex_count);
        // Clean Up
        media_glDisableClientState(GL_VERTEX_ARRAY);
        media_glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        media_glDisableClientState(GL_COLOR_ARRAY);
        media_glDisableClientState(GL_NORMAL_ARRAY);
    }
    // Finish
    self->clear();
    self->active = false;
}
static void Tesselator_draw_injection(Tesselator *self) {
    if (are_vertices_flat) {
        Tesselator_draw_injection_impl<CustomVertexFlat, &CustomTesselator::vertices_flat>(self);
    } else {
        Tesselator_draw_injection_impl<CustomVertexShaded, &CustomTesselator::vertices>(self);
    }
}

// Draw Buffer
static void drawArrayVT_injection(const int buffer, const int vertices, int vertex_size, const uint mode) {
    // Check
    if (are_vertices_flat) {
        IMPOSSIBLE();
    }
    // Setup
    vertex_size = sizeof(CustomVertexShaded);
    media_glBindBuffer(GL_ARRAY_BUFFER, buffer);
    media_glTexCoordPointer(2, GL_FLOAT, vertex_size, (void *) offsetof(CustomVertexShaded, base.uv));
    media_glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    media_glVertexPointer(3, GL_FLOAT, vertex_size, (void *) offsetof(CustomVertexShaded, base.pos));
    media_glEnableClientState(GL_VERTEX_ARRAY);
    media_glNormalPointer(GL_BYTE, vertex_size, (void *) offsetof(CustomVertexShaded, normal));
    media_glEnableClientState(GL_NORMAL_ARRAY);
    // Draw
    media_glDrawArrays(mode, 0, vertices);
    // Clean Up
    media_glDisableClientState(GL_VERTEX_ARRAY);
    media_glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    media_glDisableClientState(GL_NORMAL_ARRAY);
}

// Add Vertex
impl_template
static void Tesselator_vertex_injection_impl(Tesselator *self, const Vertex *vertex) {
    // Add To Vector
    VertexArray<Vertex> &vertices = CustomTesselator::instance.*VertexPtr;
    vertices.push_back(*vertex);
    // Convert To Triangles
    if (!enable_real_quads && self->mode == GL_QUADS) {
        int &tracker = CustomTesselator::instance.quad_to_triangle_tracker;
        if (tracker == 3) {
            tracker = 0;
        } else {
            if (tracker == 2) {
                const int last_vertex = vertices.size - 1;
                for (const int i : {-2, 0}) {
                    vertices.push_back(vertices.data[last_vertex + i]);
                }
            }
            tracker++;
        }
    }
}
static void Tesselator_vertex_injection(Tesselator *self, const float x, const float y, const float z) {
    // Create Vertex
    CustomVertexShaded vertex = {};
    vertex.base.pos = {
        (self->offset_x + x) * self->sx,
        (self->offset_y + y) * self->sy,
        self->offset_z + z
    };
    if (self->has_texture) {
        vertex.base.uv = {self->u, self->v};
    }
    if (self->has_color) {
        vertex.base.color = self->_color;
    }
    if (CustomTesselator::instance.normal) {
        vertex.normal = *CustomTesselator::instance.normal;
    }
    // Add To Array
    if (are_vertices_flat) {
        return Tesselator_vertex_injection_impl<CustomVertexFlat, &CustomTesselator::vertices_flat>(self, &vertex.base);
    } else {
        return Tesselator_vertex_injection_impl<CustomVertexShaded, &CustomTesselator::vertices>(self, &vertex);
    }
}

// Specify Normal
static void Tesselator_normal_injection(MCPI_UNUSED Tesselator *self, const float nx, const float ny, const float nz) {
    if (are_vertices_flat) {
        IMPOSSIBLE();
    }
    const signed char xx = (signed char) (nx * 127);
    const signed char yy = (signed char) (ny * 127);
    const signed char zz = (signed char) (nz * 127);
    CustomTesselator::instance.normal = xx | (yy << 8) | (zz << 16);
}

// Handle Terrain Rendering
static void Chunk_rebuild_injection(Chunk_rebuild_t original, Chunk *self) {
    // Call Original Method
    are_vertices_flat = true;
    enable_real_quads = display_lists_enabled_for_chunk_rendering;
    original(self);
    are_vertices_flat = false;
    enable_real_quads = false;
}

// Init
void _init_custom_tesselator() {
    // Replace Tesselator
    overwrite_calls(Tesselator_init, Tesselator_init_injection);
    overwrite_calls(Tesselator_clear, Tesselator_clear_injection);
    overwrite_calls(Tesselator_begin, Tesselator_begin_injection);
    overwrite_call((void *) Tesselator_end->backup, Tesselator_end, Tesselator_end_injection, true);
    overwrite_call((void *) Tesselator_draw->backup, Tesselator_draw, Tesselator_draw_injection, true);
    overwrite_call((void *) Tesselator_vertex->backup, Tesselator_vertex, Tesselator_vertex_injection, true);
    overwrite_call((void *) Tesselator_normal->backup, Tesselator_normal, Tesselator_normal_injection, true);
    overwrite_call((void *) Common_drawArrayVT->backup, Common_drawArrayVT, drawArrayVT_injection, true);
    // Render Terrain Without Normals
    overwrite_calls(Chunk_rebuild, Chunk_rebuild_injection);
}