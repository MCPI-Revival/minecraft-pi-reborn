#include <cmath>
#include <algorithm>
#include <unistd.h>

#include <libreborn/patch.h>
#include <libreborn/config.h>
#include <libreborn/env/env.h>
#include <libreborn/util/util.h>

#include <symbols/Common.h>
#include <symbols/Dimension.h>
#include <symbols/DistanceChunkSorter.h>
#include <symbols/Chunk.h>
#include <symbols/Entity.h>
#include <symbols/Tile.h>
#include <symbols/Tesselator.h>
#include <symbols/Minecraft.h>
#include <symbols/Level.h>
#include <symbols/Textures.h>
#include <symbols/Mob.h>
#include <symbols/LevelRenderer.h>
#include <symbols/CarriedTile.h>
#include <symbols/HumanoidMobRenderer.h>
#include <symbols/PlayerRenderer.h>
#include <symbols/HumanoidModel.h>
#include <symbols/ChestTileEntity.h>
#include <symbols/ContainerMenu.h>
#include <symbols/Container.h>
#include <symbols/FrustumCuller.h>
#include <symbols/GameRenderer.h>
#include <symbols/NinecraftApp.h>
#include <symbols/LevelChunk.h>

#include <GLES/gl.h>
#include <media-layer/core.h>

#include <mods/feature/feature.h>
#include <mods/display-lists/display-lists.h>
#include <mods/misc/misc.h>
#include <mods/common.h>

#include "../internal.h"

// Properly Generate Buffers
static void anGenBuffers_injection(MCPI_UNUSED Common_anGenBuffers_t original, const int32_t count, uint32_t *buffers) {
    if (!reborn_is_headless()) {
        media_glGenBuffers(count, buffers);
    }
}

// Custom Outline Color
void misc_set_nice_line_width() {
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
    static bool loaded_range = false;
    static float range[2];
    if (!loaded_range) {
        media_glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, range);
        loaded_range = true;
    }
    if (range[1] < line_width) {
        line_width = range[1];
    } else if (range[0] > line_width) {
        line_width = range[0];
    }
    // Set Line Width
    media_glLineWidth(line_width);
}
static void LevelRenderer_render_AABB_glColor4f_injection(MCPI_UNUSED GLfloat red, MCPI_UNUSED GLfloat green, MCPI_UNUSED GLfloat blue, MCPI_UNUSED GLfloat alpha) {
    // Set Color
    media_glColor4f(0, 0, 0, 0.4);
    // Set Line Width
    misc_set_nice_line_width();
}

// Java Light Ramp
static void Dimension_updateLightRamp_injection(MCPI_UNUSED Dimension_updateLightRamp_t original, Dimension *self) {
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
struct chunk_data_t {
    Chunk *chunk;
    float distance;
};
#define MAX_CHUNKS_SIZE 24336
static chunk_data_t chunk_data[MAX_CHUNKS_SIZE];
static void sort_chunks(Chunk **chunks_begin, Chunk **chunks_end, const DistanceChunkSorter sorter) {
    // Calculate Distances
    const int chunks_size = chunks_end - chunks_begin;
    if (chunks_size > MAX_CHUNKS_SIZE) {
        IMPOSSIBLE();
    }
    for (int i = 0; i < chunks_size; i++) {
        Chunk *chunk = chunks_begin[i];
        float distance = chunk->distanceToSqr((Entity *) sorter.mob);
        if (distance > 1024 && chunk->y < 64) {
            distance *= 10;
        }
        chunk_data[i].chunk = chunk;
        chunk_data[i].distance = distance;
    }

    // Sort
    std::sort(chunk_data, chunk_data + chunks_size, [](const chunk_data_t &a, const chunk_data_t &b) {
        return a.distance < b.distance;
    });
    for (int i = 0; i < chunks_size; i++) {
        chunks_begin[i] = chunk_data[i].chunk;
    }
}

// Nicer Water Rendering
static bool game_render_anaglyph_color_mask[4];
static void GameRenderer_render_glColorMask_injection(const bool red, const bool green, const bool blue, const bool alpha) {
    game_render_anaglyph_color_mask[0] = red;
    game_render_anaglyph_color_mask[1] = green;
    game_render_anaglyph_color_mask[2] = blue;
    game_render_anaglyph_color_mask[3] = alpha;
    media_glColorMask(red, green, blue, alpha);
}
static int GameRenderer_render_LevelRenderer_render_injection(LevelRenderer *self, Mob *mob, const int param_1, const float delta) {
    if (self->minecraft->options.ambient_occlusion) {
        media_glShadeModel(GL_SMOOTH);
    }
    media_glColorMask(false, false, false, false);
    const int water_chunks = self->render(mob, param_1, delta);
    media_glColorMask(true, true, true, true);
    if (self->minecraft->options.anaglyph_3d) {
        media_glColorMask(game_render_anaglyph_color_mask[0], game_render_anaglyph_color_mask[1], game_render_anaglyph_color_mask[2], game_render_anaglyph_color_mask[3]);
    }
    if (water_chunks > 0) {
        LevelRenderer_renderSameAsLast(self, delta);
    }
    media_glShadeModel(GL_FLAT);
    return water_chunks;
}

// Fix grass_carried's Bottom Texture
static int CarriedTile_getTexture2_injection(CarriedTile_getTexture2_t original, CarriedTile *self, const int face, const int metadata) {
    if (face == 0) return 2;
    return original(self, face, metadata);
}

// Fix Graphics Bug When Switching To First-Person While Sneaking
static void PlayerRenderer_render_injection(PlayerRenderer *model_renderer, Entity *entity, const float param_2, const float param_3, const float param_4, const float param_5, const float param_6) {
    HumanoidMobRenderer_render->get(false)((HumanoidMobRenderer *) model_renderer, entity, param_2, param_3, param_4, param_5, param_6);
    HumanoidModel *model = model_renderer->model;
    model->is_sneaking = false;
}

// 3D Chest Rendering
static int32_t TileRenderer_tesselateInWorld_Tile_getRenderShape_injection(Tile *tile) {
    if (tile->id == Tile::chest->id) {
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
static void ChestRenderer_render_ModelPart_render_injection(ModelPart *model_part, float scale) {
    // Start Fixing Texture
    is_rendering_chest = true;
    // Call Original Method
    model_part->render(scale);
    // Stop
    is_rendering_chest = false;
}
static void PolygonQuad_render_Tesselator_vertexUV_injection(Tesselator *self, const float x, const float y, const float z, const float u, float v) {
    // Fix Chest Texture
    if (is_rendering_chest) {
        v /= 2;
    }
    // Call Original Method
    self->vertexUV(x, y, z, u, v);
}

// Fix Invisible Chests
static void fix_chest_tile(LevelChunk *chunk, const int x, const int y, const int z, const int tile) {
    // Ensure Tile Entity Exists
    const int chest = Tile::chest->id;
    if (tile == chest) {
        chunk->getTileEntity(x, y, z);
    }
}
static void fix_chests_in_chunk(LevelChunk *chunk) {
    // Loop Through All Tiles In Chunk
    for (int x = 0; x < LevelSize::CHUNK_SIZE; x++) {
        for (int z = 0; z < LevelSize::CHUNK_SIZE; z++) {
            for (int y = 0; y < LevelSize::HEIGHT; y++) {
                const int tile = chunk->getTile(x, y, z);
                fix_chest_tile(chunk, x, y, z, tile);
            }
        }
    }
}
static void Level_loadEntities_injection(Level_loadEntities_t original, Level *self) {
    // Call Original Method
    original(self);
    // Fix Chests
    for (int chunk_x = 0; chunk_x < LevelSize::CHUNK_COUNT; chunk_x++) {
        for (int chunk_z = 0; chunk_z < LevelSize::CHUNK_COUNT; chunk_z++) {
            LevelChunk *chunk = self->getChunk(chunk_x, chunk_z);
            fix_chests_in_chunk(chunk);
        }
    }
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
static bool is_destroying_level = false;
static Level *Level_destructor_injection(Level_destructor_complete_t original, Level *self) {
    // Call Original Method
    is_destroying_level = true;
    original(self);
    is_destroying_level = false;
    return self;
}
static ContainerMenu *ContainerMenu_destructor_injection(ContainerMenu_destructor_complete_t original, ContainerMenu *container_menu) {
    // Play Animation
    Container *container = container_menu->container;
    const ChestTileEntity *tile_entity = (ChestTileEntity *) (((unsigned char *) container) - offsetof(ChestTileEntity, container));
    const bool is_client = tile_entity->is_client;
    if (!is_client && !is_destroying_level) {
        container->stopOpen();
    }

    // Call Original Method
    return original(container_menu);
}

// Vignette
static void Gui_renderProgressIndicator_injection(Gui_renderProgressIndicator_t original, Gui *self, const bool is_touch, int width, int height, float a) {
    // Render
    media_glEnable(GL_BLEND);
    self->minecraft->textures->blur = true;
    self->renderVignette(self->minecraft->camera->getBrightness(a), width, height);
    self->minecraft->textures->blur = false;
    media_glDisable(GL_BLEND);
    // Call Original Method
    original(self, is_touch, width, height, a);
}

// Increase Render Chunk Size
static void adjust_mov_shift(void *addr, const uint32_t new_shift) {
    uint32_t instruction = *(uint32_t *) addr;
    instruction &= ~0x00000f80;
    instruction |= (new_shift << 7);
    unsigned char *x = (unsigned char *) &instruction;
    patch(addr, x);
}
static int safe_log2(const int x) {
    const double y = std::log2(x);
    const int z = int(y);
    if (double(z) != y) {
        IMPOSSIBLE();
    }
    return z;
}

// Optimized Culling
static bool FrustumCuller_cubeInFrustum_injection(FrustumCuller *self, const float x1, const float y1, const float z1, const float x2, const float y2, const float z2) {
    for (int i = 0; i < 6; i++) {
        // Get Components
        const float a = self->data.data[i][0];
        const float b = self->data.data[i][1];
        const float c = self->data.data[i][2];
        const float d = self->data.data[i][3];
        // Find "Positive Vertex"
        const float x = (a > 0 ? x2 : x1) - self->x;
        const float y = (b > 0 ? y2 : y1) - self->y;
        const float z = (c > 0 ? z2 : z1) - self->z;
        // Check
        if (((a * x) + (b * y) + (c * z) + d) < 0) {
            return false;
        }
    }
    return true;
}
static bool FrustumCuller_isVisible_injection(FrustumCuller *self, const AABB &aabb) {
    return FrustumCuller_cubeInFrustum_injection(self, aabb.x1, aabb.y1, aabb.z1, aabb.x2, aabb.y2, aabb.z2);
}

// Init
void _init_misc_graphics() {
    // Disable V-Sync
    if (feature_has("Disable V-Sync", server_enabled)) {
        media_disable_vsync();
    }

    // Properly Generate Buffers
    if (feature_has("Proper OpenGL Buffer Generation", server_enabled)) {
        overwrite_calls(Common_anGenBuffers, anGenBuffers_injection);
    }

    // Replace Block Highlight With Outline
    if (feature_has("Replace Block Highlight With Outline", server_disabled)) {
        overwrite_calls(LevelRenderer_renderHitSelect, [](MCPI_UNUSED LevelRenderer_renderHitSelect_t original, LevelRenderer *self, Player *player, const HitResult &hit_result, int i, void *vp, float f) {
            self->renderHitOutline(player, hit_result, i, vp, f);
        });
        unsigned char fix_outline_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x4d830, fix_outline_patch);
        overwrite_call_manual((void *) 0x4d764, (void *) LevelRenderer_render_AABB_glColor4f_injection);
    }

    // Properly Hide Block Outline
    if (feature_has("Hide Block Outline When UI Is Hidden", server_disabled)) {
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
    if (feature_has("Optimize Chunk Sorting", server_enabled)) {
        overwrite_calls_manual((void *) 0x51fac, (void *) sort_chunks);
    }

    // Modify Entity Rendering
    _init_misc_entity_rendering();

    // Slightly Nicer Water Rendering
    if (feature_has("Improved Water Rendering", server_disabled)) {
        overwrite_call_manual((void *) 0x49ed4, (void *) GameRenderer_render_glColorMask_injection);
        overwrite_call((void *) 0x4a18c, LevelRenderer_render, GameRenderer_render_LevelRenderer_render_injection);
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
    if (feature_has("3D Chest Model", server_enabled)) {
        // Hide Old Model
        overwrite_call((void *) 0x5e830, Tile_getRenderShape, TileRenderer_tesselateInWorld_Tile_getRenderShape_injection);

        // Enable New Model
        overwrite_calls(ChestTileEntity_constructor, ChestTileEntity_injection);
        overwrite_call((void *) 0x6655c, ModelPart_render, ChestRenderer_render_ModelPart_render_injection);
        overwrite_call((void *) 0x66568, ModelPart_render, ChestRenderer_render_ModelPart_render_injection);
        overwrite_call((void *) 0x66574, ModelPart_render, ChestRenderer_render_ModelPart_render_injection);
        overwrite_call((void *) 0x4278c, Tesselator_vertexUV, PolygonQuad_render_Tesselator_vertexUV_injection);
        unsigned char chest_model_patch[4] = {0x13, 0x20, 0xa0, 0xe3}; // "mov r2, #0x13"
        patch((void *) 0x66fc8, chest_model_patch);
        unsigned char chest_color_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x66404, chest_color_patch);

        // Fix Invisible Chests
        overwrite_calls(Level_loadEntities, Level_loadEntities_injection);

        // Animation
        overwrite_calls(ContainerMenu_constructor, ContainerMenu_injection);
        overwrite_calls(Level_destructor_complete, Level_destructor_injection);
        overwrite_calls(ContainerMenu_destructor_complete, ContainerMenu_destructor_injection);
    }

    // Vignette
    if (feature_has("Render Vignette", server_disabled)) {
        overwrite_calls(Gui_renderProgressIndicator, Gui_renderProgressIndicator_injection);
    }

    // Increase Render Chunk Size
    if (feature_has("Increase Render Chunk Size", server_disabled)) {
        constexpr int chunk_size = LevelSize::CHUNK_SIZE;
        // LevelRenderer::LevelRenderer
        int a = (2 * LevelSize::SIZE) / chunk_size + 1;
        a = a * a * (LevelSize::HEIGHT / chunk_size) * 3;
        patch_address((void *) 0x4e748, (void *) a);
        a *= sizeof(int);
        patch_address((void *) 0x4e74c, (void *) a);
        // LevelRenderer::allChanged
        constexpr int b = LevelSize::HEIGHT / chunk_size;
        unsigned char render_chunk_patch_one[] = {(unsigned char) b, 0x20, 0xa0, 0xe3}; // "mov r2, #b"
        patch((void *) 0x4fbec, render_chunk_patch_one);
        adjust_mov_shift((void *) 0x4fbfc, safe_log2(b));
        const int c = safe_log2(chunk_size);
        adjust_mov_shift((void *) 0x4fbf0, c);
        adjust_mov_shift((void *) 0x4fc74, c);
        adjust_mov_shift((void *) 0x4fd60, c);
        adjust_mov_shift((void *) 0x4fd40, c);
        unsigned char render_chunk_patch_two[] = {(unsigned char) chunk_size, 0x20, 0xa0, 0xe3}; // "mov r2, #chunk_size"
        patch((void *) 0x4fc80, render_chunk_patch_two);
        // LevelRenderer::resortChunks
        adjust_mov_shift((void *) 0x4f534, c);
        constexpr int d = chunk_size / 2;
        unsigned char render_chunk_patch_three[] = {(unsigned char) d, 0x10, 0x61, 0xe2}; // "rsb r1, r1, #d"
        patch((void *) 0x4f548, render_chunk_patch_three);
        adjust_mov_shift((void *) 0x4f58c, c);
        unsigned char render_chunk_patch_four[] = {(unsigned char) d, 0x90, 0x63, 0xe2}; // "rsb r9, r3, #d"
        patch((void *) 0x4f5d0, render_chunk_patch_four);
        adjust_mov_shift((void *) 0x4f5f4, c);
        adjust_mov_shift((void *) 0x4f638, c);
        unsigned char render_chunk_patch_five[] = {(unsigned char) chunk_size, 0x90, 0x89, 0xe2}; // "add r9, r9, #chunk_size"
        patch((void *) 0x4f6c0, render_chunk_patch_five);
        unsigned char render_chunk_patch_six[] = {(unsigned char) chunk_size, 0x30, 0x83, 0xe2}; // "add r3, r3, #chunk_size"
        patch((void *) 0x4f6ec, render_chunk_patch_six);
        // LevelRenderer::setDirty
        unsigned char render_chunk_patch_seven[] = {(unsigned char) chunk_size, 0x10, 0xa0, 0xe3}; // "mov r1, #chunk_size"
        patch((void *) 0x4f1bc, render_chunk_patch_seven);
        patch((void *) 0x4f1cc, render_chunk_patch_seven);
        patch((void *) 0x4f1dc, render_chunk_patch_seven);
        patch((void *) 0x4f1ec, render_chunk_patch_seven);
        patch((void *) 0x4f1fc, render_chunk_patch_seven);
        patch((void *) 0x4f20c, render_chunk_patch_seven);
    }

    // Optimized Culling
    if (feature_has("Optimize Frustum Culling", server_disabled)) {
        overwrite_call((void *) FrustumCuller_cubeInFrustum->backup, FrustumCuller_cubeInFrustum, FrustumCuller_cubeInFrustum_injection, true);
        overwrite_call((void *) FrustumCuller_isVisible->backup, FrustumCuller_isVisible, FrustumCuller_isVisible_injection, true);
    }

    // Don't Render Game In Headless Mode
    if (reborn_is_headless()) {
        overwrite_calls(GameRenderer_render, [](MCPI_UNUSED GameRenderer_render_t original, MCPI_UNUSED GameRenderer *self, MCPI_UNUSED float param_1) {
            // Prevent Overloading
            usleep(1000);
        });
        overwrite_calls(NinecraftApp_initGLStates, nop<NinecraftApp_initGLStates_t, NinecraftApp *>);
        overwrite_calls(Gui_onConfigChanged, nop<Gui_onConfigChanged_t, Gui *, const Config &>);
        overwrite_calls(LevelRenderer_generateSky, nop<LevelRenderer_generateSky_t, LevelRenderer *>);
        overwrite_calls_manual((void *) 0xdd40, (void *) nop<GLsizei, const GLuint *>); // glDeleteBuffers
    }
}