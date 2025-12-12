#include <symbols/Tesselator.h>

#include "internal.h"

// Specify Color
static void clamp_color(int &x) {
    x = std::max(0, std::min(255, x));
}
__attribute__((hot)) static void Tesselator_color_injection(MCPI_UNUSED Tesselator *self, int r, int g, int b, int a) {
    CustomTesselator &t = advanced_tesselator_get();
    if (t.no_color) {
        return;
    }
    clamp_color(r);
    clamp_color(g);
    clamp_color(b);
    clamp_color(a);
    t.color = (a << 24) | (b << 16) | (g << 8) | (r << 0);
}
__attribute__((hot)) static void Tesselator_colorABGR_injection(MCPI_UNUSED Tesselator *self, const uint color) {
    CustomTesselator &t = advanced_tesselator_get();
    if (t.no_color) {
        return;
    }
    t.color = color;
}
static uint Tesselator_getColor_injection(MCPI_UNUSED Tesselator *self) {
    const std::optional<uint> color = advanced_tesselator_get().color;
    return color.value_or(0);
}
__attribute__((hot)) static void Tesselator_noColor_injection(MCPI_UNUSED Tesselator *self) {
    advanced_tesselator_get().no_color = true;
}
__attribute__((hot)) static void Tesselator_enableColor_injection(MCPI_UNUSED Tesselator *self) {
    advanced_tesselator_get().no_color = false;
}

// Specify Texture
__attribute__((hot)) static void Tesselator_tex_injection(MCPI_UNUSED Tesselator *self, const float u, const float v) {
    CustomTesselator &t = advanced_tesselator_get();
    t.uv = {
        .u = u,
        .v = v
    };
}

// Specify Normal
__attribute__((hot)) static void Tesselator_normal_injection(MCPI_UNUSED Tesselator *self, const float nx, const float ny, const float nz) {
    CustomTesselator &t = advanced_tesselator_get();
    if (t.are_vertices_flat) {
        IMPOSSIBLE();
    }
    const signed char xx = (signed char) (nx * 127);
    const signed char yy = (signed char) (ny * 127);
    const signed char zz = (signed char) (nz * 127);
    t.normal = xx | (yy << 8) | (zz << 16);
}

// Specify Scale
static void Tesselator_scale2d_injection(MCPI_UNUSED Tesselator *self, const float scale_x, const float scale_y) {
    CustomTesselator &t = advanced_tesselator_get();
    t.scale_x *= scale_x;
    t.scale_y *= scale_y;
}
static void Tesselator_resetScale_injection(MCPI_UNUSED Tesselator *self) {
    advanced_tesselator_get().reset_scale();
}

// Specify Offset
static void Tesselator_offset_vec3_injection(MCPI_UNUSED Tesselator *self, const Vec3 &x) {
    advanced_tesselator_get().offset = x;
}
static void Tesselator_addOffset_vec3_injection(MCPI_UNUSED Tesselator *self, const Vec3 &x) {
    Vec3 &offset = advanced_tesselator_get().offset;
    offset.x += x.x;
    offset.y += x.y;
    offset.z += x.z;
}
static void Tesselator_offset_injection(MCPI_UNUSED Tesselator *self, const float x, const float y, const float z) {
    Tesselator_offset_vec3_injection(nullptr, {x, y, z});
}
static void Tesselator_addOffset_injection(MCPI_UNUSED Tesselator *self, const float x, const float y, const float z) {
    Tesselator_addOffset_vec3_injection(nullptr, {x, y, z});
}

// Add Vertex
TEMPLATE
__attribute__((hot, always_inline)) static inline void Tesselator_vertex_injection_impl(CustomTesselator &t, const Vertex *vertex) {
    // Add To Vector
    VertexArray<Vertex> &vertices = t.*VertexPtr;
    vertices.push_back(*vertex);
    // Convert To Triangles
    if (!t.enable_real_quads && t.mode == GL_QUADS) {
        int &tracker = t.quad_to_triangle_tracker;
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
__attribute__((hot)) static void Tesselator_vertex_injection(MCPI_UNUSED Tesselator *self, const float x, const float y, const float z) {
    // Create Vertex
    CustomTesselator &t = advanced_tesselator_get();
    CustomVertexShaded vertex = {};
    vertex.base.pos = {
        (t.offset.x + x) * t.scale_x,
        (t.offset.y + y) * t.scale_y,
        t.offset.z + z
    };
    if (t.uv.has_value()) {
        vertex.base.uv = t.uv.value();
    }
    if (t.color.has_value()) {
        vertex.base.color = t.color.value();
    }
    if (t.normal.has_value()) {
        vertex.normal = t.normal.value();
    }
    // Add To Array
    if (t.are_vertices_flat) {
        return Tesselator_vertex_injection_impl<CustomVertexFlat, &CustomTesselator::vertices_flat>(t, &vertex.base);
    } else {
        return Tesselator_vertex_injection_impl<CustomVertexShaded, &CustomTesselator::vertices>(t, &vertex);
    }
}
__attribute__((hot)) static void Tesselator_vertexUV_injection(MCPI_UNUSED Tesselator *self, const float x, const float y, const float z, const float u, const float v) {
    Tesselator_tex_injection(nullptr, u, v);
    Tesselator_vertex_injection(nullptr, x, y, z);
}

// Patch
void _tesselator_init_vertex() {
    replace(Tesselator_color);
    replace(Tesselator_colorABGR);
    replace(Tesselator_getColor);
    replace(Tesselator_noColor);
    replace(Tesselator_enableColor);
    replace(Tesselator_tex);
    replace(Tesselator_normal);
    replace(Tesselator_scale2d);
    replace(Tesselator_resetScale);
    replace(Tesselator_offset);
    replace(Tesselator_offset_vec3);
    replace(Tesselator_addOffset);
    replace(Tesselator_addOffset_vec3);
    replace(Tesselator_vertex);
    replace(Tesselator_vertexUV);
}