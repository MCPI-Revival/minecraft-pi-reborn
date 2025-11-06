#include <libreborn/patch.h>

#include <GLES/gl.h>

#include <mods/display-lists/display-lists.h>

#include "CachedLevelSource.h"
#include "internal.h"

// Create/Delete Display Lists
static void create_lists(LevelRenderer *self) {
    self->display_lists = media_glGenLists(self->num_buffers);
}
static void delete_lists(const LevelRenderer *self) {
    media_glDeleteLists(self->display_lists, self->num_buffers);
}
static LevelRenderer *LevelRenderer_constructor_injection(LevelRenderer_constructor_t original, LevelRenderer *self, Minecraft *minecraft) {
    // Call Original Method
    original(self, minecraft);
    // Create Display Lists
    create_lists(self);
    return self;
}
static void LevelRenderer_onGraphicsReset_injection(LevelRenderer_onGraphicsReset_t original, LevelRenderer *self) {
    // Re-Create Display Lists
    create_lists(self);
    // Call Original Method
    original(self);
}
static LevelRenderer *LevelRenderer_destructor_injection(LevelRenderer_destructor_complete_t original, LevelRenderer *self) {
    // Delete Display Lists
    delete_lists(self);
    // Call Original Method
    return original(self);
}

// Cached Level Source
static CachedLevelSource &get_cached_level_source() {
    static CachedLevelSource source;
    return source;
}
Level *get_level_from_cached_level_source(const LevelSource *level_source) {
    const CachedLevelSource &cached_level_source = get_cached_level_source();
    if (level_source == cached_level_source.self) {
        return cached_level_source.level;
    } else {
        return nullptr;
    }
}

// Build Chunk
static void Chunk_rebuild_injection(MCPI_UNUSED Chunk_rebuild_t original, Chunk *self) {
    // Prepare
    if (!self->dirty) {
        return;
    }
    const int x0 = self->x;
    const int y0 = self->y;
    const int z0 = self->z;
    const int x1 = self->x + self->width;
    const int y1 = self->y + self->height;
    const int z1 = self->z + self->depth;
    for (bool &empty : self->is_layer_empty) {
        empty = true;
    }
    self->is_empty = true;
    LevelChunk::touchedSky = false;
    Level *level = self->level;
    Tesselator &t = Tesselator::instance;

    // Get Chunk Information
    CachedLevelSource &source = get_cached_level_source();
    source.level = level;
    source._cache(x0, y0, z0, x1, y1, z1);
    if (!source.should_render) {
        // Empty Chunk
        self->touched_sky = false;
        self->built = true;
        return;
    }
    TileRenderer *tile_renderer = TileRenderer::allocate();
    tile_renderer->constructor(source.self);

    // Render
    constexpr int num_layers = sizeof(self->is_layer_empty) / sizeof(bool);
    bool should_render_layer[num_layers] = {true, false, false};
    for (int layer = 0; layer < num_layers; layer++) {
        if (!should_render_layer[layer]) {
            continue;
        }
        // Render Layer
        bool started = false;
        bool rendered = false;
        for (int y = y0; y < y1; y++) {
            for (int z = z0; z < z1; z++) {
                for (int x = x0; x < x1; x++) {
                    if (!source._should_render(x, y, z)) {
                        continue;
                    }
                    // Render Tile
                    const int tile_id = source.getTile(x, y, z);
                    Tile *tile = Tile::tiles[tile_id];
                    const int tile_layer = tile->getRenderLayer();
                    if (tile_layer > layer) {
                        should_render_layer[tile_layer] = true;
                    } else if (tile_layer == layer) {
                        if (!started) {
                            // Start Rendering
                            started = true;
                            media_glNewList(self->display_lists + layer, GL_COMPILE);
                            t.begin(GL_QUADS);
                        }
                        rendered |= tile_renderer->tesselateInWorld(tile, x, y, z);
                    }
                }
            }
        }
        // Save Layer
        if (started) {
            t.draw();
            media_glEndList();
            if (rendered) {
                self->is_layer_empty[layer] = false;
                self->is_empty = false;
            }
        }
    }

    // Clean Up
    ::operator delete(tile_renderer);
    self->touched_sky = LevelChunk::touchedSky;
    self->built = true;
}

// Render
#define MAX_DISPLAY_LISTS 4096
static GLuint display_lists[MAX_DISPLAY_LISTS];
static int num_display_lists = 0;
static void display_lists_renderSameAsLast(const LevelRenderer *self, const float b) {
    // Prepare Offset
    const Mob *camera = self->minecraft->camera;
    const float x = camera->old_x + ((camera->x - camera->old_x) * b);
    const float y = camera->old_y + ((camera->y - camera->old_y) * b);
    const float z = camera->old_z + ((camera->z - camera->old_z) * b);
    media_glPushMatrix();
    media_glTranslatef(-x, -y, -z);

    // Render
    media_glCallLists(num_display_lists, GL_UNSIGNED_INT, display_lists);

    // Clean Up
    media_glPopMatrix();
}
static int LevelRenderer_renderChunks_injection(MCPI_UNUSED LevelRenderer_renderChunks_t original, const LevelRenderer *self, const int start, const int end, const int a, const float b) {
    // Gather Display Lists
    num_display_lists = 0;
    for (int i = start; i < end; i++) {
        Chunk *chunk = self->chunks[i];
        // Check If Chunk Is Visible
        if (!chunk->is_layer_empty[a] && chunk->visible) {
            const GLuint list = chunk->getList(a);
            if (list > 0) {
                display_lists[num_display_lists++] = list;
            }
        }
    }

    // Draw
    display_lists_renderSameAsLast(self, b);

    // Return
    return num_display_lists;
}

// API
bool display_lists_enabled_for_chunk_rendering = false;
void LevelRenderer_renderSameAsLast(LevelRenderer *self, const float delta) {
    if (display_lists_enabled_for_chunk_rendering) {
        display_lists_renderSameAsLast(self, delta);
    } else {
        self->render_list.render();
    }
}

// Init
void _init_display_lists_chunks() {
    // Create/Delete Display Lists
    overwrite_calls(LevelRenderer_constructor, LevelRenderer_constructor_injection);
    overwrite_calls(LevelRenderer_onGraphicsReset, LevelRenderer_onGraphicsReset_injection);
    overwrite_calls(LevelRenderer_destructor_complete, LevelRenderer_destructor_injection);

    // Disable Creating/Deleting Buffers
    for (const uint32_t addr : {0x4e6f8, 0x4e51c, 0x4e564}) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) addr, nop_patch);
    }

    // Build Chunks
    overwrite_calls(Chunk_rebuild, Chunk_rebuild_injection);

    // Render Chunks
    display_lists_enabled_for_chunk_rendering = true;
    overwrite_calls(LevelRenderer_renderChunks, LevelRenderer_renderChunks_injection);
}