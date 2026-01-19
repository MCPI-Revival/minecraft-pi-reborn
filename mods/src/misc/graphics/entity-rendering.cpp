#include "../internal.h"

#include <symbols/EntityRenderer.h>
#include <symbols/Entity.h>
#include <symbols/Tile.h>
#include <symbols/Tesselator.h>
#include <symbols/EntityRenderDispatcher.h>
#include <symbols/Minecraft.h>
#include <symbols/Level.h>
#include <symbols/Textures.h>
#include <symbols/Mob.h>
#include <symbols/Mth.h>
#include <symbols/ArmorScreen.h>

#include <GLES/gl.h>
#include <libreborn/util/util.h>
#include <libreborn/patch.h>

#include <mods/feature/feature.h>
#include <mods/common.h>

// Fire Rendering
static void render_fire(EntityRenderer *self, Entity *entity, const float x, float y, const float z) {
    // Check If Entity Is On Fire
    if (!entity->isOnFire()) {
        return;
    }
    // Here Be Decompiled Code
    y -= entity->height_offset;
    const int texture = Tile::fire->texture;
    const int xt = texture % AtlasSize::TILE_COUNT;
    const int yt = texture / AtlasSize::TILE_COUNT;
    media_glPushMatrix();
    media_glTranslatef(x, y, z);
    const float s = entity->hitbox_width * 1.4f;
    media_glScalef(s, s, s);
    self->bindTexture("terrain.png");
    Tesselator &t = Tesselator::instance;
    float r = 0.5f;
    float h = entity->hitbox_height / s;
    float yo = entity->y - entity->height_offset - entity->hitbox.y1;
    float player_rot_y = EntityRenderer::entityRenderDispatcher->player_rot_y;
    if (EntityRenderer::entityRenderDispatcher->minecraft->options.third_person == 2) {
        // Handle Front-Facing
        player_rot_y -= 180.f;
    }
    media_glRotatef(-player_rot_y, 0, 1, 0);
    media_glTranslatef(0, 0, -0.3f + float(int(h)) * 0.02f);
    media_glColor4f(1, 1, 1, 1);
    float zo = 0;
    int ss = 0;
    t.begin(GL_QUADS);
    while (h > 0) {
        constexpr float xo = 0.0f;
        float u0;
        float u1;
        float v0;
        float v1;
        if (ss % 2 == 0) {
            u0 = float(xt) / AtlasSize::TILE_COUNT;
            u1 = (float(xt) + 1) / AtlasSize::TILE_COUNT;
            v0 = float(yt) / AtlasSize::TILE_COUNT;
            v1 = (float(yt) + 1) / AtlasSize::TILE_COUNT;
        } else {
            u0 = float(xt) / AtlasSize::TILE_COUNT;
            u1 = (float(xt) + 1) / AtlasSize::TILE_COUNT;
            v0 = (float(yt) + 1) / AtlasSize::TILE_COUNT;
            v1 = (float(yt) + 2) / AtlasSize::TILE_COUNT;
        }
        if (ss / 2 % 2 == 0) {
            std::swap(u1, u0);
        }
        t.vertexUV(r - xo, 0 - yo, zo, u1, v1);
        t.vertexUV(-r - xo, 0 - yo, zo, u0, v1);
        t.vertexUV(-r - xo, 1.4f - yo, zo, u0, v0);
        t.vertexUV(r - xo, 1.4f - yo, zo, u1, v0);
        h -= 0.45f;
        yo -= 0.45f;
        r *= 0.9f;
        zo += 0.03f;
        ss++;
    }
    t.draw();
    media_glPopMatrix();
}

// Entity Shadows
static void render_shadow_tile(Tile *tile, const float x, const float y, const float z, const int xt, const int yt, const int zt, const float pow, const float r, const float xo, const float yo, const float zo) {
    Tesselator &t = Tesselator::instance;
    if (!tile->isCubeShaped()) {
        return;
    }
    float a = ((pow - (y - (float(yt) + yo)) / 2) * 0.5f) * EntityRenderer::entityRenderDispatcher->level->getBrightness(xt, yt, zt);
    if (a < 0) {
        return;
    } else if (a > 1) {
        a = 1;
    }
    t.color(255, 255, 255, int(a * 255));
    const float x0 = float(xt) + tile->x1 + xo;
    const float x1 = float(xt) + tile->x2 + xo;
    const float y0 = float(yt) + tile->y1 + yo + 1.0f / 64.0f;
    const float z0 = float(zt) + tile->z1 + zo;
    const float z1 = float(zt) + tile->z2 + zo;
    const float u0 = (x - x0) / 2 / r + 0.5f;
    const float u1 = (x - x1) / 2 / r + 0.5f;
    const float v0 = (z - z0) / 2 / r + 0.5f;
    const float v1 = (z - z1) / 2 / r + 0.5f;
    t.vertexUV(x0, y0, z0, u0, v0);
    t.vertexUV(x0, y0, z1, u0, v1);
    t.vertexUV(x1, y0, z1, u1, v1);
    t.vertexUV(x1, y0, z0, u1, v0);
}
static void render_shadow(const EntityRenderer *self, Entity *entity, const float x, const float y, const float z, const float a) {
    // Calculate Power
    float pow = 0;
    if (self->shadow_radius > 0) {
        const float dist = EntityRenderer::entityRenderDispatcher->distanceToSqr(entity->x, entity->y, entity->z);
        pow = (1 - dist / (16.0f * 16.0f)) * self->shadow_strength;
    }
    if (pow <= 0) {
        return;
    }
    // Render
    media_glEnable(GL_BLEND);
    media_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Textures *textures = EntityRenderer::entityRenderDispatcher->textures;
    textures->loadAndBindTexture("misc/shadow.png");
    media_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    media_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    Level *level = EntityRenderer::entityRenderDispatcher->level;
    media_glDepthMask(false);
    float r = self->shadow_radius;
    if (entity->isMob()) {
        Mob *mob = (Mob *) entity;
        if (mob->isBaby()) {
            r *= 0.5f;
        }
    }
    const float ex = entity->old_x + (entity->x - entity->old_x) * a;
    const float ey = entity->old_y + (entity->y - entity->old_y) * a + entity->getShadowHeightOffs() - entity->height_offset;
    const float ez = entity->old_z + (entity->z - entity->old_z) * a;
    const int x0 = Mth::floor(ex - r);
    const int x1 = Mth::floor(ex + r);
    const int y0 = Mth::floor(ey - r);
    const int y1 = Mth::floor(ey);
    const int z0 = Mth::floor(ez - r);
    const int z1 = Mth::floor(ez + r);
    const float xo = x - ex;
    const float yo = y - ey;
    const float zo = z - ez;
    Tesselator &tt = Tesselator::instance;
    tt.begin(GL_QUADS);
    for (int xt = x0; xt <= x1; xt++) {
        for (int yt = y0; yt <= y1; yt++) {
            for (int zt = z0; zt <= z1; zt++) {
                const int t = level->getTile(xt, yt - 1, zt);
                if (t > 0 && level->getRawBrightness(xt, yt, zt) > 3) {
                    render_shadow_tile(
                        Tile::tiles[t],
                        x, y + entity->getShadowHeightOffs() - entity->height_offset, z,
                        xt, yt, zt,
                        pow, r,
                        xo, yo + entity->getShadowHeightOffs() - entity->height_offset, zo
                    );
                }
            }
        }
    }
    tt.draw();
    media_glColor4f(1, 1, 1, 1);
    media_glDisable(GL_BLEND);
    media_glDepthMask(true);
}
static void EntityRenderDispatcher_assign_injection(EntityRenderDispatcher_assign_t original, EntityRenderDispatcher *self, const uchar entity_id, EntityRenderer *renderer) {
    // Modify Shadow Size
    float new_radius;
    switch (entity_id) {
        // Humanoid/Player
        case 16:
        case 3: {
            new_radius = 0.5f;
            break;
        }
        // Sheep/Cow/Pig
        case 9:
        case 7:
        case 8: {
            new_radius = 0.7f;
            break;
        }
        // Chicken
        case 6: {
            new_radius = 0.3f;
            break;
        }
        // Default
        default: {
            new_radius = renderer->shadow_radius;
        }
    }
    renderer->shadow_radius = new_radius;
    // Call Original Method
    original(self, entity_id, renderer);
}

// Modify Entity Rendering
static bool is_lighting_enabled = false;
#define TRACK_LIGHTING(mode, val) \
    HOOK(mode, void, (const GLenum cap)) { \
        if (cap == GL_LIGHTING) { \
            is_lighting_enabled = val; \
        } \
        real_##mode()(cap); \
    }
TRACK_LIGHTING(media_glEnable, true)
TRACK_LIGHTING(media_glDisable, false)
#undef TRACK_LIGHTING
static bool should_render_fire;
static bool should_render_shadows;
static void EntityRenderDispatcher_render_EntityRenderer_render_injection(EntityRenderer *self, Entity *entity, float x, float y, float z, float rot, float unknown) {
    // Call Original Method
    self->render(entity, x, y, z, rot, unknown);
    // Render Shadow
    if (should_render_shadows) {
        render_shadow(self, entity, x, y, z, unknown);
    }
    // Render Fire
    if (should_render_fire) {
        const bool was_lighting_enabled = is_lighting_enabled;
        media_glDisable(GL_LIGHTING);
        render_fire(self, entity, x, y, z);
        if (was_lighting_enabled) {
            media_glEnable(GL_LIGHTING);
        }
    }
}

// Hide Shadow In Armor Screen
static void ArmorScreen_renderPlayer_injection(ArmorScreen_renderPlayer_t original, ArmorScreen *self, float param_1, float param_2) {
    should_render_shadows = false;
    original(self, param_1, param_2);
    should_render_shadows = true;
}

// Init
void _init_misc_entity_rendering() {
    bool hijack_entity_rendering = false;
    // Fire
    should_render_fire = feature_has("Render Fire In Third-Person", server_disabled);
    if (should_render_fire) {
        hijack_entity_rendering = true;
    }
    // Shadows
    should_render_shadows = feature_has("Render Entity Shadows", server_disabled);
    if (should_render_shadows) {
        overwrite_calls(EntityRenderDispatcher_assign, EntityRenderDispatcher_assign_injection);
        overwrite_calls(ArmorScreen_renderPlayer, ArmorScreen_renderPlayer_injection);
        hijack_entity_rendering = true;
    }
    // Actually Patch The Code
    if (hijack_entity_rendering) {
        overwrite_call((void *) 0x606c0, EntityRenderer_render, EntityRenderDispatcher_render_EntityRenderer_render_injection);
    }
}