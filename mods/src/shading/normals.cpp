#include <symbols/minecraft.h>
#include <libreborn/libreborn.h>

#include "shading-internal.h"

// PolygonQuad
Vec3 vector_to(const Vec3 &a, const Vec3 &b) {
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}
Vec3 vector_cross(const Vec3 &a, const Vec3 &b) {
    return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
static void PolygonQuad_render_injection(PolygonQuad_render_t original, PolygonQuad *self, Tesselator &t, float scale, int buffer) {
    // Set Normal
    const Vec3 v0 = vector_to(self->vertices[1].pos, self->vertices[0].pos);
    const Vec3 v1 = vector_to(self->vertices[1].pos, self->vertices[2].pos);
    const Vec3 n = vector_cross(v1, v0).normalized();
    t.normal(n.x, n.y, n.z);
    // Call Original Method
    original(self, t, scale, buffer);
}

// Init
void _init_normals() {
    overwrite_calls(PolygonQuad_render, PolygonQuad_render_injection);
}