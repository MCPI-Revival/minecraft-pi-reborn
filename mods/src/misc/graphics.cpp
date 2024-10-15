#include <cmath>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <GLES/gl.h>
#include <media-layer/core.h>

#include <mods/feature/feature.h>
#include <mods/multidraw/multidraw.h>

#include "misc-internal.h"

// Properly Generate Buffers
static void anGenBuffers_injection(__attribute__((unused)) Common_anGenBuffers_t original, const int32_t count, uint32_t *buffers) {
    if (!reborn_is_headless()) {
        glGenBuffers(count, buffers);
    }
}

// Custom Outline Color
static void LevelRenderer_render_AABB_glColor4f_injection(__attribute__((unused)) GLfloat red, __attribute__((unused)) GLfloat green, __attribute__((unused)) GLfloat blue, __attribute__((unused)) GLfloat alpha) {
    // Set Color
    glColor4f(0, 0, 0, 0.4);

    // Find Line Width
    const char *custom_line_width = getenv(MCPI_BLOCK_OUTLINE_WIDTH_ENV);
    float line_width;
    if (custom_line_width != nullptr) {
        // Custom
        line_width = strtof(custom_line_width, nullptr);
    } else {
        // Guess
        line_width = 1.5f / Gui::InvGuiScale;
    }
    // Clamp Line Width
    float range[2];
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, range);
    if (range[1] < line_width) {
        line_width = range[1];
    } else if (range[0] > line_width) {
        line_width = range[0];
    }
    // Set Line Width
    glLineWidth(line_width);
}

// Java Light Ramp
static void Dimension_updateLightRamp_injection(__attribute__((unused)) Dimension_updateLightRamp_t original, Dimension *self) {
    // https://github.com/ReMinecraftPE/mcpe/blob/d7a8b6baecf8b3b050538abdbc976f690312aa2d/source/world/level/Dimension.cpp#L92-L105
    for (int i = 0; i <= 15; i++) {
        const float f1 = 1.0f - (((float) i) / 15.0f);
        self->light_ramp[i] = ((1.0f - f1) / (f1 * 3.0f + 1.0f)) * (1.0f - 0.1f) + 0.1f;
        // Default Light Ramp:
        // float fVar4 = 1.0 - ((float) i * 0.0625);
        // self->light_ramp[i] = ((1.0 - fVar4) / (fVar4 * 3.0 + 1.0)) * 0.95 + 0.15;
    }
}

// Sort Chunks
struct chunk_data {
    Chunk *chunk;
    float distance;
};
#define MAX_CHUNKS_SIZE 24336
static chunk_data data[MAX_CHUNKS_SIZE];
static void sort_chunks(Chunk **chunks_begin, Chunk **chunks_end, const DistanceChunkSorter sorter) {
    // Calculate Distances
    const int chunks_size = chunks_end - chunks_begin;
    if (chunks_size > MAX_CHUNKS_SIZE) {
        IMPOSSIBLE();
    }
    for (int i = 0; i < chunks_size; i++) {
        Chunk *chunk = chunks_begin[i];
        float distance = chunk->distanceToSqr((Entity *) sorter.mob);
        if ((1024.0 <= distance) && chunk->y < 0x40) {
            distance *= 10.0;
        }
        data[i].chunk = chunk;
        data[i].distance = distance;
    }

    // Sort
    std::sort(data, data + chunks_size, [](chunk_data &a, chunk_data &b) {
        return a.distance < b.distance;
    });
    for (int i = 0; i < chunks_size; i++) {
        chunks_begin[i] = data[i].chunk;
    }
}

// Fire Rendering
static void render_fire(EntityRenderer *self, Entity *entity, const float x, float y, const float z) {
    // Check If Entity Is On Fire
    if (!entity->isOnFire()) {
        return;
    }
    // Here Be Decompiled Code
    y -= entity->height_offset;
    const int texture = Tile::fire->texture;
    const int xt = (texture & 0xf) << 4;
    const int yt = texture & 0xf0;
    glPushMatrix();
    glTranslatef(x, y, z);
    const float s = entity->hitbox_width * 1.4f;
    glScalef(s, s, s);
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
    glRotatef(-player_rot_y, 0, 1, 0);
    glTranslatef(0, 0, -0.3f + float(int(h)) * 0.02f);
    glColor4f(1, 1, 1, 1);
    float zo = 0;
    int ss = 0;
    t.begin(7);
    while (h > 0) {
        constexpr float xo = 0.0f;
        float u0;
        float u1;
        float v0;
        float v1;
        if (ss % 2 == 0) {
            u0 = float(xt) / 256.0f;
            u1 = (float(xt) + 15.99f) / 256.0f;
            v0 = float(yt) / 256.0f;
            v1 = (float(yt) + 15.99f) / 256.0f;
        } else {
            u0 = float(xt) / 256.0f;
            u1 = (float(xt) + 15.99f) / 256.0f;
            v0 = (float(yt) + 16) / 256.0f;
            v1 = (float(yt) + 16 + 15.99f) / 256.0f;
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
    glPopMatrix();
}

// Entity Shadows
static void render_shadow_tile(Tile *tile, const float x, const float y, const float z, int xt, int yt, int zt, const float pow, const float r, const float xo, const float yo, const float zo) {
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
    float x0 = float(xt) + tile->x1 + xo;
    float x1 = float(xt) + tile->x2 + xo;
    float y0 = float(yt) + tile->y1 + yo + 1.0f / 64.0f;
    float z0 = float(zt) + tile->z1 + zo;
    float z1 = float(zt) + tile->z2 + zo;
    float u0 = (x - x0) / 2 / r + 0.5f;
    float u1 = (x - x1) / 2 / r + 0.5f;
    float v0 = (z - z0) / 2 / r + 0.5f;
    float v1 = (z - z1) / 2 / r + 0.5f;
    t.vertexUV(x0, y0, z0, u0, v0);
    t.vertexUV(x0, y0, z1, u0, v1);
    t.vertexUV(x1, y0, z1, u1, v1);
    t.vertexUV(x1, y0, z0, u1, v0);
}
static void render_shadow(const EntityRenderer *self, Entity *entity, float x, float y, float z, const float a) {
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Textures *textures = EntityRenderer::entityRenderDispatcher->textures;
    textures->loadAndBindTexture("misc/shadow.png");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    Level *level = EntityRenderer::entityRenderDispatcher->level;
    glDepthMask(false);
    const float r = self->shadow_radius;
    const float ex = entity->old_x + (entity->x - entity->old_x) * a;
    float ey = entity->old_y + (entity->y - entity->old_y) * a + entity->getShadowHeightOffs() - entity->height_offset;
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
    tt.begin(7);
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
    glColor4f(1, 1, 1, 1);
    glDisable(GL_BLEND);
    glDepthMask(true);
}
static void EntityRenderDispatcher_assign_injection(EntityRenderDispatcher_assign_t original, EntityRenderDispatcher *self, const uchar entity_id, EntityRenderer *renderer) {
    // Modify Shadow Size
    float new_radius;
    switch (entity_id) {
        case 16:
        case 3: {
            new_radius = 0.5f;
            break;
        }
        case 9:
        case 7:
        case 8: {
            new_radius = 0.7f;
            break;
        }
        case 6: {
            new_radius = 0.3f;
            break;
        }
        default: {
            new_radius = renderer->shadow_radius;
        }
    }
    renderer->shadow_radius = new_radius;
    // Call Original Method
    original(self, entity_id, renderer);
}

// Modify Entity Rendering
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
        render_fire(self, entity, x, y, z);
    }
}

// Nicer Water Rendering
static bool game_render_anaglyph_color_mask[4];
static void GameRenderer_render_glColorMask_injection(const bool red, const bool green, const bool blue, const bool alpha) {
    game_render_anaglyph_color_mask[0] = red;
    game_render_anaglyph_color_mask[1] = green;
    game_render_anaglyph_color_mask[2] = blue;
    game_render_anaglyph_color_mask[3] = alpha;
    glColorMask(red, green, blue, alpha);
}
static int GameRenderer_render_LevelRenderer_render_injection(LevelRenderer *self, Mob *mob, int param_1, float delta) {
    glColorMask(false, false, false, false);
    const int water_chunks = self->render(mob, param_1, delta);
    glColorMask(true, true, true, true);
    if (self->minecraft->options.anaglyph_3d) {
        glColorMask(game_render_anaglyph_color_mask[0], game_render_anaglyph_color_mask[1], game_render_anaglyph_color_mask[2], game_render_anaglyph_color_mask[3]);
    }
    if (water_chunks > 0) {
        LevelRenderer_renderSameAsLast(self, delta);
    }
    return water_chunks;
}

// Fix grass_carried's Bottom Texture
static int CarriedTile_getTexture2_injection(CarriedTile_getTexture2_t original, CarriedTile *self, const int face, const int metadata) {
    if (face == 0) return 2;
    return original(self, face, metadata);
}

// Fix Graphics Bug When Switching To First-Person While Sneaking
static void PlayerRenderer_render_injection(PlayerRenderer *model_renderer, Entity *entity, const float param_2, const float param_3, const float param_4, const float param_5, const float param_6) {
    HumanoidMobRenderer_vtable_base->render((HumanoidMobRenderer *) model_renderer, entity, param_2, param_3, param_4, param_5, param_6);
    HumanoidModel *model = model_renderer->model;
    model->is_sneaking = false;
}

// 3D Chests
static int32_t Tile_getRenderShape_injection(Tile *tile) {
    if (tile == Tile::chest) {
        // Don't Render "Simple" Chest Model
        return -1;
    } else {
        // Call Original Method
        return tile->getRenderShape();
    }
}
static ChestTileEntity *ChestTileEntity_injection(ChestTileEntity_constructor_t original, ChestTileEntity *tile_entity) {
    // Call Original Method
    original(tile_entity);

    // Enable Renderer
    tile_entity->renderer_id = 1;

    // Return
    return tile_entity;
}
static bool is_rendering_chest = false;
static void ModelPart_render_injection(ModelPart *model_part, float scale) {
    // Start
    is_rendering_chest = true;

    // Call Original Method
    model_part->render(scale);

    // Stop
    is_rendering_chest = false;
}
static void Tesselator_vertexUV_injection(Tesselator_vertexUV_t original, Tesselator *tesselator, const float x, const float y, const float z, const float u, float v) {
    // Fix Chest Texture
    if (is_rendering_chest) {
        v /= 2;
    }

    // Call Original Method
    original(tesselator, x, y, z, u, v);
}
static bool ChestTileEntity_shouldSave_injection(__attribute__((unused)) ChestTileEntity_shouldSave_t original, __attribute__((unused)) ChestTileEntity *tile_entity) {
    return true;
}

// Animated 3D Chest
static ContainerMenu *ContainerMenu_injection(ContainerMenu_constructor_t original, ContainerMenu *container_menu, Container *container, const int32_t param_1) {
    // Call Original Method
    original(container_menu, container, param_1);

    // Play Animation
    const ChestTileEntity *tile_entity = (ChestTileEntity *) (((unsigned char *) container) - offsetof(ChestTileEntity, container));
    const bool is_client = tile_entity->is_client;
    if (!is_client) {
        container->startOpen();
    }

    // Return
    return container_menu;
}
static ContainerMenu *ContainerMenu_destructor_injection(ContainerMenu_destructor_complete_t original, ContainerMenu *container_menu) {
    // Play Animation
    Container *container = container_menu->container;
    const ChestTileEntity *tile_entity = (ChestTileEntity *) (((unsigned char *) container) - offsetof(ChestTileEntity, container));
    const bool is_client = tile_entity->is_client;
    if (!is_client) {
        container->stopOpen();
    }

    // Call Original Method
    return original(container_menu);
}

// 3D Dropped Items
static bool disable_hand_positioning = false;
static void ItemInHandRenderer_renderItem_glTranslatef_injection(const float x, const float y, const float z) {
    if (disable_hand_positioning) {
        glPopMatrix();
        glPushMatrix();
    } else {
        glTranslatef(x, y, z);
    }
}
static void ItemRenderer_render_injection(ItemRenderer_render_t original, ItemRenderer *self, Entity *entity, const float x, const float y, const float z, const float a, const float b) {
    // Get Item
    const ItemEntity *item_entity = (ItemEntity *) entity;
    ItemInstance item = item_entity->item;
    // Check If Item Is Tile
    if (item.id < 256 && TileRenderer::canRender(Tile::tiles[item.id]->getRenderShape())) {
        // Call Original Method
        original(self, entity, x, y, z, a, b);
    } else {
        // 3D Item
        self->random.setSeed(187);
        glPushMatrix();

        // Count
        int count;
        if (item.count < 2) {
            count = 1;
        } else if (item.count < 16) {
            count = 2;
        } else if (item.count < 32) {
            count = 3;
        } else {
            count = 4;
        }

        // Bob
        const float age = float(item_entity->age) + b;
        const float bob = (Mth::sin((age / 10.0f) + item_entity->bob_offset) * 0.1f) + 0.1f;
        glTranslatef(x, y + bob, z);

        // Scale
        glScalef(0.5f, 0.5f, 0.5f);

        // Spin
        const float spin = ((age / 20.0f) + item_entity->bob_offset) * float(180.0f / M_PI);
        glRotatef(spin, 0, 1, 0);

        // Position
        constexpr float xo = 0.5f;
        constexpr float yo = 0.25f;
        constexpr float width = 1 / 16.0f;
        constexpr float margin = 0.35f / 16.0f;
        constexpr float zo = width + margin;
        glTranslatef(-xo, -yo, -((zo * float(count)) / 2));

        // Draw
        disable_hand_positioning = true;
        for (int i = 0; i < count; i++) {
            glTranslatef(0, 0, zo);
            EntityRenderer::entityRenderDispatcher->item_renderer->renderItem(nullptr, &item);
        }
        disable_hand_positioning = false;

        // Finish
        glPopMatrix();
    }
}

// Vignette
static void Gui_renderProgressIndicator_injection(Gui_renderProgressIndicator_t original, Gui *self, const bool is_touch, int width, int height, float a) {
    // Render
    glEnable(GL_BLEND);
    self->minecraft->textures->blur = true;
    self->renderVignette(self->minecraft->player->getBrightness(a), width, height);
    self->minecraft->textures->blur = false;
    glDisable(GL_BLEND);
    // Call Original Method
    original(self, is_touch, width, height, a);
}

// Init
void _init_misc_graphics() {
    // Disable V-Sync
    if (feature_has("Disable V-Sync", server_enabled)) {
        media_disable_vsync();
    }

    // Force EGL
    if (feature_has("Force EGL", server_disabled)) {
        media_force_egl();
    }

    // Properly Generate Buffers
    if (feature_has("Proper OpenGL Buffer Generation", server_enabled)) {
        overwrite_calls(Common_anGenBuffers, anGenBuffers_injection);
    }

    // Replace Block Highlight With Outline
    if (feature_has("Replace Block Highlight With Outline", server_disabled)) {
        overwrite_calls(LevelRenderer_renderHitSelect, [](__attribute__((unused)) LevelRenderer_renderHitSelect_t original, LevelRenderer *self, Player *player, const HitResult &hit_result, int i, void *vp, float f) {
            self->renderHitOutline(player, hit_result, i, vp, f);
        });
        unsigned char fix_outline_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x4d830, fix_outline_patch);
        overwrite_call((void *) 0x4d764, (void *) LevelRenderer_render_AABB_glColor4f_injection);
    }

    // Properly Hide Block Outline
    if (feature_has("Hide Block Outline When GUI Is Hidden", server_disabled)) {
        overwrite_calls(LevelRenderer_renderHitSelect, [](LevelRenderer_renderHitSelect_t original, LevelRenderer *self, Player *player, const HitResult &hit_result, const int i, void *vp, const float f) {
            if (!self->minecraft->options.hide_gui) {
                original(self, player, hit_result, i, vp, f);
            }
        });
    }

    // Java Light Ramp
    if (feature_has("Use Java Beta 1.3 Light Ramp", server_disabled)) {
        overwrite_calls(Dimension_updateLightRamp, Dimension_updateLightRamp_injection);
    }

    // Replace 2011 std::sort With Optimized(TM) Code
    if (feature_has("Optimized Chunk Sorting", server_enabled)) {
        overwrite_calls_manual((void *) 0x51fac, (void *) sort_chunks);
    }

    // Modify Entity Rendering
    overwrite_call((void *) 0x606c0, (void *) EntityRenderDispatcher_render_EntityRenderer_render_injection);
    should_render_fire = feature_has("Render Fire In Third-Person", server_disabled);
    should_render_shadows = feature_has("Render Entity Shadows", server_disabled);
    if (should_render_shadows) {
        overwrite_calls(EntityRenderDispatcher_assign, EntityRenderDispatcher_assign_injection);
    }

    // Slightly Nicer Water Rendering
    if (feature_has("Improved Water Rendering", server_disabled)) {
        overwrite_call((void *) 0x49ed4, (void *) GameRenderer_render_glColorMask_injection);
        overwrite_call((void *) 0x4a18c, (void *) GameRenderer_render_LevelRenderer_render_injection);
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x4a12c, nop_patch);
    }

    // Fix grass_carried's Bottom Texture
    if (feature_has("Fix Carried Grass's Bottom Texture", server_disabled)) {
        overwrite_calls(CarriedTile_getTexture2, CarriedTile_getTexture2_injection);
    }

    // Fix Graphics Bug When Switching To First-Person While Sneaking
    if (feature_has("Fix Switching Perspective While Sneaking", server_disabled)) {
        patch_vtable(PlayerRenderer_render, PlayerRenderer_render_injection);
    }

    // 3D Chests
    if (feature_has("3D Chest Model", server_disabled)) {
        overwrite_call((void *) 0x5e830, (void *) Tile_getRenderShape_injection);
        overwrite_calls(ChestTileEntity_constructor, ChestTileEntity_injection);
        overwrite_call((void *) 0x6655c, (void *) ModelPart_render_injection);
        overwrite_call((void *) 0x66568, (void *) ModelPart_render_injection);
        overwrite_call((void *) 0x66574, (void *) ModelPart_render_injection);
        overwrite_calls(Tesselator_vertexUV, Tesselator_vertexUV_injection);
        unsigned char chest_model_patch[4] = {0x13, 0x20, 0xa0, 0xe3}; // "mov r2, #0x13"
        patch((void *) 0x66fc8, chest_model_patch);
        unsigned char chest_color_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x66404, chest_color_patch);

        // Animation
        overwrite_calls(ContainerMenu_constructor, ContainerMenu_injection);
        overwrite_calls(ContainerMenu_destructor_complete, ContainerMenu_destructor_injection);
    }
    if (feature_has("Always Save Chest Tile Entities", server_enabled)) {
        overwrite_calls(ChestTileEntity_shouldSave, ChestTileEntity_shouldSave_injection);
    }

    // 3D Dropped Items
    if (feature_has("3D Dropped Items", server_disabled)) {
        overwrite_calls(ItemRenderer_render, ItemRenderer_render_injection);
        overwrite_call((void *) 0x4bf34, (void *) ItemInHandRenderer_renderItem_glTranslatef_injection);
    }

    // Vignette
    if (feature_has("Render Vignette", server_disabled)) {
        overwrite_calls(Gui_renderProgressIndicator, Gui_renderProgressIndicator_injection);
    }

    // Don't Render Game In Headless Mode
    if (reborn_is_headless()) {
        overwrite_calls(GameRenderer_render, nop<GameRenderer_render_t, GameRenderer *, float>);
        overwrite_calls(NinecraftApp_initGLStates, nop<NinecraftApp_initGLStates_t, NinecraftApp *>);
        overwrite_calls(Gui_onConfigChanged, nop<Gui_onConfigChanged_t, Gui *, const Config &>);
        overwrite_calls(LevelRenderer_generateSky, nop<LevelRenderer_generateSky_t, LevelRenderer *>);
    }
}