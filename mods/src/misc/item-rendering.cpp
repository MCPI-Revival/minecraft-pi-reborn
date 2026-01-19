#include <ranges>
#include <cmath>

#include <GLES/gl.h>
#include <libreborn/patch.h>

#include <symbols/ItemInHandRenderer.h>
#include <symbols/GameRenderer.h>
#include <symbols/ItemRenderer.h>
#include <symbols/ItemEntity.h>
#include <symbols/TileRenderer.h>
#include <symbols/Tile.h>
#include <symbols/Mth.h>
#include <symbols/EntityRenderDispatcher.h>
#include <symbols/Mob.h>
#include <symbols/Minecraft.h>
#include <symbols/Tesselator.h>
#include <symbols/Texture.h>
#include <symbols/Textures.h>

#include <mods/feature/feature.h>
#include <mods/shading/shading.h>
#include <mods/common.h>

#include "internal.h"

// Common Functions
static int get_icon(ItemInstance *item, Mob *mob) {
    if (mob) {
        return mob->getItemInHandIcon(item, 0);
    } else {
        return item->getIcon();
    }
}
static bool is_tile(const ItemInstance *item) {
    return item->id < TILE_ITEM_BARRIER;
}
static constexpr int atlas_size = AtlasSize::TILE_COUNT;

// Fix Invalid ItemInHandRenderer Cache
static std::unordered_map<int, ItemInHandRenderer_RenderCall> held_item_cache;
static void ItemInHandRenderer_renderItem_injection_1(ItemInHandRenderer_renderItem_t original, ItemInHandRenderer *self, Mob *mob, ItemInstance *item) {
    // Create Hash
    int hash = get_icon(item, mob);
    if (!is_tile(item)) {
        constexpr int atlas_end = atlas_size * atlas_size;
        hash += atlas_end;
    }

    // Create Buffer If Needed
    if (!held_item_cache.contains(hash)) {
        ItemInHandRenderer_RenderCall call = {};
        call.chunk.constructor();
        media_glGenBuffers(1, &call.chunk.buffer);
        call.id = -1;
        held_item_cache.insert({hash, call});
    }

    // Call Original Method
    ItemInHandRenderer_RenderCall &slot = held_item_cache.at(hash);
    ItemInHandRenderer_RenderCall &selected_call = self->render_calls[0];
    selected_call = slot;
    original(self, mob, item);

    // Store Result
    if (selected_call.id < 0) {
        IMPOSSIBLE();
    }
    slot = selected_call;
}
static void ItemInHandRenderer_onGraphicsReset_injection(ItemInHandRenderer_onGraphicsReset_t original, ItemInHandRenderer *self) {
    // Clear Cache
    held_item_cache.clear();
    // Call Original Method
    original(self);
}
static void *GameRenderer_destructor_injection(GameRenderer_destructor_t original, GameRenderer *self, const int unknown) {
    // Free Buffers
    // This is placed inside GameRenderer's destructor
    // because ItemInHandRenderer does not have one.
    for (const ItemInHandRenderer_RenderCall &call : held_item_cache | std::views::values) {
        media_glDeleteBuffers(1, &call.chunk.buffer);
    }
    held_item_cache.clear();
    // Call Original Method
    return original(self, unknown);
}

// 3D Dropped Items
static bool disable_hand_positioning = false;
static void ItemInHandRenderer_renderItem_glTranslatef_injection(const float x, const float y, const float z) {
    if (disable_hand_positioning) {
        media_glPopMatrix();
        media_glPushMatrix();
    } else {
        media_glTranslatef(x, y, z);
    }
}
static constexpr float one_pixel = 1.0f / 16.0f;
static constexpr float item_depth = one_pixel;
static void ItemRenderer_render_injection(ItemRenderer_render_t original, ItemRenderer *self, Entity *entity, const float x, const float y, const float z, const float a, const float b) {
    // Get Item
    ItemEntity *item_entity = (ItemEntity *) entity;
    ItemInstance *item = &item_entity->item;
    // Check If Item Is Tile
    if (is_tile(item) && TileRenderer::canRender(Tile::tiles[item->id]->getRenderShape())) {
        // Call Original Method
        original(self, entity, x, y, z, a, b);
    } else {
        // 3D Item
        self->random.setSeed(187);
        media_glPushMatrix();

        // Count
        int count;
        if (item->count < 2) {
            count = 1;
        } else if (item->count < 16) {
            count = 2;
        } else if (item->count < 32) {
            count = 3;
        } else {
            count = 4;
        }

        // Bob
        const float age = float(item_entity->age) + b;
        const float bob = (Mth::sin((age / 10.0f) + item_entity->bob_offset) * 0.1f) + 0.1f;
        media_glTranslatef(x, y + bob, z);

        // Scale
        media_glScalef(0.5f, 0.5f, 0.5f);

        // Spin
        const float spin = ((age / 20.0f) + item_entity->bob_offset) * float(180.0f / M_PI);
        media_glRotatef(spin, 0, 1, 0);

        // Position
        constexpr float xo = 0.5f;
        constexpr float yo = 0.25f;
        constexpr float width = item_depth;
        constexpr float margin = 0.35f * one_pixel;
        const float full_width = (width * float(count)) + (margin * float(count - 1));
        media_glTranslatef(-xo, -yo, -(full_width / 2));

        // Draw
        disable_hand_positioning = true;
        for (int i = 0; i < count; i++) {
            media_glTranslatef(0, 0, width);
            EntityRenderer::entityRenderDispatcher->item_renderer->renderItem(nullptr, item);
            media_glTranslatef(0, 0, margin);
        }
        disable_hand_positioning = false;

        // Finish
        media_glPopMatrix();
    }
}

// Fix High-Resolution 3D Items
static int item_icon_to_render;
static int atlas_tile_size;
static void ItemInHandRenderer_renderItem_injection_2(ItemInHandRenderer_renderItem_t original, ItemInHandRenderer *self, Mob *mob, ItemInstance *item) {
    // Get Item Icon
    item_icon_to_render = get_icon(item, mob);

    // Get Atlas Tile Size
    constexpr int default_atlas_tile_size = AtlasSize::TILE_SIZE;
    Textures *textures = self->minecraft->textures;
    const char *texture_name = is_tile(item) ? "terrain.png" : "gui/items.png";
    const uint texture_id = textures->loadTexture(texture_name, true);
    const Texture *texture = textures->getTemporaryTextureData(texture_id);
    atlas_tile_size = texture ? (texture->width / atlas_size) : default_atlas_tile_size;

    // Call Original Method
    original(self, mob, item);
}
static RenderChunk ItemInHandRenderer_renderItem_Tesselator_end_injection(Tesselator *t, const bool use_given_buffer, const int buffer) {
    // Clear Old Vertices
    t->clear();

    // Tessellate 3D Item
    const float up = float(item_icon_to_render % atlas_size);
    const float vp = float(int(item_icon_to_render / atlas_size));
    const float u1 = up / atlas_size;
    const float u0 = (up + 1) / atlas_size;
    const float v0 = vp / atlas_size;
    const float v1 = (vp + 1) / atlas_size;
    constexpr float depth = item_depth;
    constexpr float size = 1;
    safe_normal(0, 0, 1);
    t->vertexUV(0, 0, 0, u0, v1);
    t->vertexUV(size, 0, 0, u1, v1);
    t->vertexUV(size, size, 0, u1, v0);
    t->vertexUV(0, size, 0, u0, v0);
    safe_normal(0, 0, -1);
    t->vertexUV(0, size, -depth, u0, v0);
    t->vertexUV(size, size, -depth, u1, v0);
    t->vertexUV(size, 0, -depth, u1, v1);
    t->vertexUV(0, 0, -depth, u0, v1);
    const float atlas_size_in_pixels = float(atlas_size * atlas_tile_size);
    std::vector<float> p_values;
    for (int i = 0; i < atlas_tile_size; i++) {
        const float p = float(i) / float(atlas_tile_size);
        p_values.push_back(p);
    }
    const float one_real_pixel = 1.0f / float(atlas_tile_size);
    const float texel_offset = 0.5f / atlas_size_in_pixels;
    safe_normal(-1, 0, 0);
    for (const float p : p_values) {
        const float u = u0 + ((u1 - u0) * p) - texel_offset;
        const float x = p * size;
        t->vertexUV(x, 0, -depth, u, v1);
        t->vertexUV(x, 0, 0, u, v1);
        t->vertexUV(x, size, 0, u, v0);
        t->vertexUV(x, size, -depth, u, v0);
    }
    safe_normal(1, 0, 0);
    for (const float p : p_values) {
        const float u = u0 + ((u1 - u0) * p) - texel_offset;
        const float x = (p + one_real_pixel) * size;
        t->vertexUV(x, size, -depth, u, v0);
        t->vertexUV(x, size, 0, u, v0);
        t->vertexUV(x, 0, 0, u, v1);
        t->vertexUV(x, 0, -depth, u, v1);
    }
    safe_normal(0, 1, 0);
    for (const float p : p_values) {
        const float v = v1 + ((v0 - v1) * p) - texel_offset;
        const float y = (p + one_real_pixel) * size;
        t->vertexUV(0, y, 0, u0, v);
        t->vertexUV(size, y, 0, u1, v);
        t->vertexUV(size, y, -depth, u1, v);
        t->vertexUV(0, y, -depth, u0, v);
    }
    safe_normal(0, -1, 0);
    for (const float p : p_values) {
        const float v = v1 + ((v0 - v1) * p) - texel_offset;
        const float y = p * size;
        t->vertexUV(size, y, 0, u1, v);
        t->vertexUV(0, y, 0, u0, v);
        t->vertexUV(0, y, -depth, u0, v);
        t->vertexUV(size, y, -depth, u1, v);
    }

    // Render
    return t->end(use_given_buffer, buffer);
}

// Init
void _init_misc_item_rendering() {
    // Fix Invalid ItemInHandRenderer Cache
    if (feature_has("Fix Held Item Caching", server_disabled)) {
        // Always Use The First Render Call Object
        // This Will Be Populated Using A Custom Cache
        unsigned char use_first_render_call_patch[4] = {0x00, 0x40, 0xa0, 0xe3}; // "mov r4, #0x0"
        patch((void *) 0x4b834, use_first_render_call_patch);
        unsigned char disable_old_cache_invalidation_check_patch[4] = {0x03, 0x00, 0x53, 0xe1}; // "cmp r3, r3"
        patch((void *) 0x4b968, disable_old_cache_invalidation_check_patch);
        // Manually Handle OpenGL Buffer Allocation
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        for (const uint32_t addr : {0x4b6fc, 0x4cdc8}) {
            patch((void *) addr, nop_patch);
        }
        overwrite_calls(ItemInHandRenderer_onGraphicsReset, ItemInHandRenderer_onGraphicsReset_injection);
        overwrite_calls(GameRenderer_destructor, GameRenderer_destructor_injection);
        // Implement A Custom (Improved) Held Item Cache
        overwrite_calls(ItemInHandRenderer_renderItem, ItemInHandRenderer_renderItem_injection_1);
    }

    // 3D Dropped Items
    if (feature_has("3D Dropped Items", server_disabled)) {
        overwrite_calls(ItemRenderer_render, ItemRenderer_render_injection);
        overwrite_call_manual((void *) 0x4bf34, (void *) ItemInHandRenderer_renderItem_glTranslatef_injection);
    }

    // High-Resolution 3D Items
    if (feature_has("Fix 3D Item Rendering With High-Resolution Textures", server_disabled)) {
        overwrite_calls(ItemInHandRenderer_renderItem, ItemInHandRenderer_renderItem_injection_2);
        overwrite_call((void *) 0x4beac, Tesselator_end, ItemInHandRenderer_renderItem_Tesselator_end_injection);
    }
}