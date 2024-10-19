#include <optional>
#include <cstddef>
#include <algorithm>
#include <cxxabi.h>

#include <GLES/gl.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/multidraw/multidraw.h>
#include "shading-internal.h"

// Structures
struct UV {
    float u;
    float v;
};
struct CustomVertex {
    Vec3 pos;
    UV uv;
    GLuint color;
    GLuint normal;
};
struct CustomTesselator {
    int vertex_count;
    CustomVertex *vertices;
    std::optional<uint32_t> normal;
    int quad_to_triangle_tracker;

    static CustomTesselator instance;
};
CustomTesselator CustomTesselator::instance;

// Setup Vertex Array
static void Tesselator_clear_injection(Tesselator_clear_t original, Tesselator *self) {
    if (original) {
        original(self);
    }
    CustomTesselator::instance.vertex_count = 0;
    CustomTesselator::instance.quad_to_triangle_tracker = 0;
    CustomTesselator::instance.normal.reset();
}
#define MAX_VERTICES 524288
static void Tesselator_init_injection(Tesselator_init_t original, Tesselator *self) {
    original(self);
    CustomTesselator::instance.vertices = new CustomVertex[MAX_VERTICES];
    Tesselator_clear_injection(nullptr, nullptr);
}

// Handle Tesselation Start
static void Tesselator_begin_injection(Tesselator_begin_t original, Tesselator *self, const int mode) {
    original(self, mode);
    if (!self->void_begin_end) {
        Tesselator_clear_injection(nullptr, nullptr);
    }
}

// Drawing
static GLuint get_next_buffer() {
    Tesselator::instance.next_buffer_id++;
    Tesselator::instance.next_buffer_id %= Tesselator::instance.buffer_count;
    const GLuint out = Tesselator::instance.buffers[Tesselator::instance.next_buffer_id];
    return out;
}
static RenderChunk Tesselator_end_injection(Tesselator *self, const bool use_given_buffer, const int buffer) {
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
    out.vertices = CustomTesselator::instance.vertex_count;
    if (out.vertices > 0) {
        out.buffer = use_given_buffer ? buffer : get_next_buffer();
        glBindBuffer(GL_ARRAY_BUFFER, out.buffer);
        glBufferData(GL_ARRAY_BUFFER, out.vertices * sizeof(CustomVertex), CustomTesselator::instance.vertices, GL_STATIC_DRAW);
    }
    // Finish
    self->clear();
    self->active = false;
    return out;
}
static void Tesselator_draw_injection(Tesselator *self) {
    // Check
    if (!self->active) {
        IMPOSSIBLE();
    }
    if (self->void_begin_end) {
        return;
    }
    // Render
    const int vertices = CustomTesselator::instance.vertex_count;
    if (vertices > 0) {
        const GLuint buffer = get_next_buffer();
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, vertices * sizeof(CustomVertex), CustomTesselator::instance.vertices, GL_STATIC_DRAW);
        if (self->has_texture) {
            glTexCoordPointer(2, GL_FLOAT, sizeof(CustomVertex), (void *) offsetof(CustomVertex, uv));
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        if (self->has_color) {
            glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(CustomVertex), (void *) offsetof(CustomVertex, color));
            glEnableClientState(GL_COLOR_ARRAY);
        }
        if (CustomTesselator::instance.normal) {
            glNormalPointer(GL_BYTE, sizeof(CustomVertex), (void *) offsetof(CustomVertex, normal));
            glEnableClientState(GL_NORMAL_ARRAY);
        }
        glVertexPointer(3, GL_FLOAT, sizeof(CustomVertex), (void *) offsetof(CustomVertex, pos));
        glEnableClientState(GL_VERTEX_ARRAY);
        int mode = self->mode;
        if (mode == GL_QUADS) {
            mode = GL_TRIANGLES;
        }
        glDrawArrays(mode, 0, vertices);
        glDisableClientState(GL_VERTEX_ARRAY);
        if (self->has_texture) {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        if (self->has_color) {
            glDisableClientState(GL_COLOR_ARRAY);
        }
        if (CustomTesselator::instance.normal) {
            glDisableClientState(GL_NORMAL_ARRAY);
        }
    }
    // Finish
    self->clear();
    self->active = false;
}
static void drawArrayVT_injection(const int buffer, const int vertices, int vertex_size, const uint mode) {
    vertex_size = sizeof(CustomVertex);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glTexCoordPointer(2, GL_FLOAT, vertex_size, (void *) offsetof(CustomVertex, uv));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, vertex_size, (void *) offsetof(CustomVertex, pos));
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_BYTE, sizeof(CustomVertex), (void *) offsetof(CustomVertex, normal));
    glEnableClientState(GL_NORMAL_ARRAY);
    glDrawArrays(mode, 0, vertices);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

// Add Vertex
static void Tesselator_vertex_injection(const Tesselator *self, const float x, const float y, const float z) {
    CustomVertex &vertex = CustomTesselator::instance.vertices[CustomTesselator::instance.vertex_count++];
    vertex.pos = {
        (self->offset_x + x) * self->sx,
        (self->offset_y + y) * self->sy,
        self->offset_z + z
    };
    if (self->has_texture) {
        vertex.uv = {self->u, self->v};
    }
    if (self->has_color) {
        vertex.color = self->_color;
    }
    if (CustomTesselator::instance.normal) {
        vertex.normal = *CustomTesselator::instance.normal;
    }
    // Convert To Triangles
    if (self->mode == GL_QUADS) {
        int &tracker = CustomTesselator::instance.quad_to_triangle_tracker;
        if (tracker == 3) {
            tracker = 0;
        } else {
            if (tracker == 2) {
                const int last_vertex = CustomTesselator::instance.vertex_count - 1;
                for (const int i : {-2, 0}) {
                    CustomTesselator::instance.vertices[CustomTesselator::instance.vertex_count++] = CustomTesselator::instance.vertices[last_vertex + i];
                }
            }
            tracker++;
        }
    }
}

// Specify Normal
static void Tesselator_normal_injection(__attribute__((unused)) Tesselator *self, const float nx, const float ny, const float nz) {
    const signed char xx = (signed char) (nx * 127);
    const signed char yy = (signed char) (ny * 127);
    const signed char zz = (signed char) (nz * 127);
    CustomTesselator::instance.normal = xx | (yy << 8) | (zz << 16);
}

static void explore(const __cxxabiv1::__class_type_info *info) {
    INFO("Test: %s", info->name());
    const __cxxabiv1::__si_class_type_info *a = dynamic_cast<const __cxxabiv1::__si_class_type_info *>(info);
    if (a) {
        explore(a->__base_type);
    } else {
        const __cxxabiv1::__vmi_class_type_info *b = dynamic_cast<const __cxxabiv1::__vmi_class_type_info *>(info);
        if (b) {
            for (unsigned int i = 0; i < b->__base_count; i++) {
                explore(b->__base_info[i].__base_type);
            }
        }
    }
}

// Init
void _init_custom_tesselator() {
    multidraw_vertex_size = sizeof(CustomVertex);
    overwrite_calls(Tesselator_init, Tesselator_init_injection);
    overwrite_calls(Tesselator_clear, Tesselator_clear_injection);
    overwrite_calls(Tesselator_begin, Tesselator_begin_injection);
    overwrite_call((void *) Tesselator_end->backup, (void *) Tesselator_end_injection, true);
    overwrite_call((void *) Tesselator_draw->backup, (void *) Tesselator_draw_injection, true);
    overwrite_call((void *) Tesselator_vertex->backup, (void *) Tesselator_vertex_injection, true);
    overwrite_call((void *) Tesselator_normal->backup, (void *) Tesselator_normal_injection, true);
    overwrite_call((void *) Common_drawArrayVT->backup, (void *) drawArrayVT_injection, true);

    const std::type_info *info = *(((std::type_info **) Cow_vtable_base) - 1);
    const __cxxabiv1::__si_class_type_info *info2 = dynamic_cast<const __cxxabiv1::__si_class_type_info *>(info);
    explore(info2);
}