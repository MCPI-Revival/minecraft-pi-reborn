#include <cstdint>

#include <symbols/Vec3.h>
#include <symbols/PolygonQuad.h>
#include <symbols/Tesselator.h>
#include <symbols/Tile.h>
#include <symbols/TileRenderer.h>

#include <libreborn/patch.h>

#include <mods/shading/shading.h>

#include "internal.h"

// PolygonQuad
static Vec3 vector_to(const Vec3 &a, const Vec3 &b) {
    return Vec3(b.x - a.x, b.y - a.y, b.z - a.z);
}
static Vec3 vector_cross(const Vec3 &a, const Vec3 &b) {
    return Vec3((a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x));
}
static void PolygonQuad_render_injection(PolygonQuad_render_t original, PolygonQuad *self, Tesselator &t, const float scale, const int buffer) {
    // Set Normal
    const Vec3 v0 = vector_to(self->vertices[1].pos, self->vertices[0].pos);
    const Vec3 v1 = vector_to(self->vertices[1].pos, self->vertices[2].pos);
    const Vec3 n = vector_cross(v1, v0).normalized();
    t.normal(n.x, n.y, n.z);
    // Call Original Method
    original(self, t, scale, buffer);
}

// Specify Normal Before Vertex
#define add_normal_before(type, name) \
    template <int nx, int ny, int nz> \
    static decltype(auto) type##_##name##_injection(type *self, auto... args) { \
        Tesselator::instance.normal(nx, ny, nz); \
        return self->name(std::forward<decltype(args)>(args)...); \
    } \
    template <int nx, int ny, int nz> \
    static void add_normal_before_##name(uint32_t addr) { \
        std::remove_pointer_t<decltype(type##_##name)>::ptr_type func = type##_##name##_injection<nx, ny, nz>; \
        overwrite_call((void *) addr, type##_##name, func); \
    }
add_normal_before(Tesselator, vertexUV)
add_normal_before(Tesselator, vertex)
add_normal_before(Tile, getTexture2)
add_normal_before(TileRenderer, tesselateCrossTexture)
add_normal_before(Tile, updateDefaultShape)

// Safe Way For Other Mods To Use Normals
static bool normals_enabled = false;
void safe_normal(const float x, const float y, const float z) {
    if (normals_enabled) {
        Tesselator::instance.normal(x, y, z);
    }
}

// Init
void _init_normals() {
    normals_enabled = true;
    // PolygonQuad::render
    overwrite_calls(PolygonQuad_render, PolygonQuad_render_injection);
    // ItemInHandRenderer::renderItem
    add_normal_before_vertexUV<0, 0, 1>(0x4bb4c);
    add_normal_before_vertexUV<0, 0, -1>(0x4bbbc);
    add_normal_before_vertexUV<-1, 0, 0>(0x4bc50);
    add_normal_before_vertexUV<1, 0, 0>(0x4bcf0);
    add_normal_before_vertexUV<0, 1, 0>(0x4bd90);
    add_normal_before_vertexUV<0, -1, 0>(0x4be28);
    // TileRenderer::renderTile
    add_normal_before_getTexture2<0, -1, 0>(0x5dd2c);
    add_normal_before_getTexture2<0, 1, 0>(0x5dd60);
    add_normal_before_getTexture2<0, 0, -1>(0x5dd94);
    add_normal_before_getTexture2<0, 0, 1>(0x5ddc8);
    add_normal_before_getTexture2<-1, 0, 0>(0x5ddfc);
    add_normal_before_getTexture2<1, 0, 0>(0x5de30);
    add_normal_before_tesselateCrossTexture<0, -1, 0>(0x5de7c);
    add_normal_before_updateDefaultShape<0, -1, 0>(0x5dea0);
    add_normal_before_getTexture2<0, -1, 0>(0x5df38);
    add_normal_before_getTexture2<0, 1, 0>(0x5df68);
    add_normal_before_getTexture2<0, 0, -1>(0x5dfac);
    add_normal_before_getTexture2<0, 0, 1>(0x5e004);
    add_normal_before_getTexture2<-1, 0, 0>(0x5e05c);
    add_normal_before_getTexture2<1, 0, 0>(0x5e0b4);
    add_normal_before_getTexture2<0, -1, 0>(0x5e1e4);
    add_normal_before_getTexture2<0, 1, 0>(0x5e218);
    add_normal_before_getTexture2<0, 0, -1>(0x5e248);
    add_normal_before_getTexture2<0, 0, 1>(0x5e278);
    add_normal_before_getTexture2<-1, 0, 0>(0x5e2a8);
    add_normal_before_getTexture2<1, 0, 0>(0x5e2d8);
    add_normal_before_getTexture2<0, -1, 0>(0x5e408);
    add_normal_before_getTexture2<0, 1, 0>(0x5e43c);
    add_normal_before_getTexture2<0, 0, -1>(0x5e46c);
    add_normal_before_getTexture2<0, 0, 1>(0x5e49c);
    add_normal_before_getTexture2<-1, 0, 0>(0x5e4cc);
    add_normal_before_getTexture2<1, 0, 0>(0x5e4fc);
    add_normal_before_getTexture2<0, -1, 0>(0x5e60c);
    add_normal_before_getTexture2<0, 1, 0>(0x5e640);
    add_normal_before_getTexture2<0, 0, -1>(0x5e670);
    add_normal_before_getTexture2<0, 0, 1>(0x5e6a0);
    add_normal_before_getTexture2<-1, 0, 0>(0x5e6d0);
    add_normal_before_getTexture2<1, 0, 0>(0x5e700);
    // ItemRenderer::render
    add_normal_before_vertexUV<0, 1, 0>(0x63394);
    // ItemSpriteRenderer::render
    add_normal_before_vertexUV<0, 1, 0>(0x63fd0);
    // MobRenderer::renderNameTag
    add_normal_before_vertex<0, 1, 0>(0x64918);
    // PaintingRenderer::renderPainting
    add_normal_before_vertexUV<0, 0, -1>(0x64c94);
    add_normal_before_vertexUV<0, 0, 1>(0x64d04);
    add_normal_before_vertexUV<0, 1, 0>(0x64d74);
    add_normal_before_vertexUV<0, -1, 0>(0x64de4);
    add_normal_before_vertexUV<-1, 0, 0>(0x64e54);
    add_normal_before_vertexUV<1, 0, 0>(0x64ec4);
}