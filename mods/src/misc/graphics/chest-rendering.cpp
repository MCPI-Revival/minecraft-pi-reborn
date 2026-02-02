#include "../internal.h"

#include <libreborn/patch.h>

#include <mods/common.h>

#include <symbols/Tile.h>
#include <symbols/Tesselator.h>
#include <symbols/ChestTileEntity.h>
#include <symbols/ContainerMenu.h>
#include <symbols/Container.h>
#include <symbols/LevelChunk.h>
#include <symbols/ModelPart.h>
#include <symbols/Level.h>

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

// Init
void _init_misc_chest_rendering() {
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