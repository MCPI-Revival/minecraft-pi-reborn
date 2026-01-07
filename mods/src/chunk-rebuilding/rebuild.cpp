#include "thread/worker.h"
#include "thread/messages.h"
#include "rebuild.h"

#include <libreborn/patch.h>

#include <symbols/LevelRenderer.h>
#include <symbols/GameRenderer.h>
#include <symbols/NinecraftApp.h>
#include <symbols/Chunk.h>
#include <symbols/Level.h>
#include <symbols/Tesselator.h>

#include <mods/feature/feature.h>
#include <mods/tesselator/tesselator.h>
#include <mods/init/init.h>

// Start/Stop Thread
static void LevelRenderer_setLevel_injection(LevelRenderer_setLevel_t original, LevelRenderer *self, Level *level) {
    // Call Original Method
    original(self, level);

    // Start/Stop Thread
    if (level) {
        _start_chunk_rebuild_thread(level);
    } else {
        _stop_chunk_rebuild_thread();
    }
}
static LevelRenderer *LevelRenderer_destructor_injection(LevelRenderer_destructor_complete_t original, LevelRenderer *self) {
    _stop_chunk_rebuild_thread();
    return original(self);
}
static void NinecraftApp_teardown_injection(NinecraftApp_teardown_t original, NinecraftApp *self) {
    _stop_chunk_rebuild_thread();
    original(self);
}

// Trigger Chunk Rebuild
static void Chunk_rebuild_injection(MCPI_UNUSED Chunk_rebuild_t original, Chunk *self) {
    if (!self->dirty) {
        return;
    }
    Level *level = self->level;
    chunk_rebuild_data *data = new chunk_rebuild_data;
    data->seed = level->getSeed();
    data->chunk = self;
    data->x0 = self->x;
    data->y0 = self->y;
    data->z0 = self->z;
    data->x1 = self->x + self->width;
    data->y1 = self->y + self->height;
    data->z1 = self->z + self->depth;
    data->source._prepare_cache(level, data->x0, data->y0, data->z0, data->x1, data->y1, data->z1);
    _start_chunk_rebuild(self, data);
}

// Configure Rendering
void _configure_tesselator_for_chunk_rebuild(const bool enable) {
    CustomTesselator &t = advanced_tesselator_get();
    t.are_vertices_flat = enable;
}

// Render A Rebuilt Chunk
static void render_rebuilt_chunk(const rebuilt_chunk_data *chunk_data, LevelRenderer *level_renderer) {
    // Render Chunk
    Chunk *chunk = chunk_data->chunk;
    chunk_data->is_empty.apply(chunk);
    chunk->touched_sky = chunk_data->touched_sky;
    chunk->built = true;
    for (int layer = 0; layer < num_layers; layer++) {
        // Skip Empty Layers
        if (chunk->is_layer_empty[layer]) {
            continue;
        }

        // Prepare To Draw
        _configure_tesselator_for_chunk_rebuild(true);
        Tesselator &t = Tesselator::instance;
        t.begin(GL_QUADS);

        // Set Vertices
        advanced_tesselator_get().vertices_flat.receive(chunk_data->vertices[layer]);

        // Enable Colors And Texturing
        t.color(0, 0, 0, 0);
        t.tex(0, 0);

        // Draw
        const GLuint buffer = chunk->buffers[layer];
        RenderChunk *render_chunk = chunk->getRenderChunk(layer);
        *render_chunk = t.end(true, int(buffer));
        _configure_tesselator_for_chunk_rebuild(false);
    }

    // Trigger Another Rebuild If Sky Color Has Changed
    if (!chunk->isDirty() && chunk->touched_sky) {
        const int new_sky_darken = level_renderer->level->sky_darken; // Current Sky Color
        const int old_sky_darken = chunk_data->old_sky_darken; // Sky Color When The Rebuild Was Triggerred
        if (new_sky_darken != old_sky_darken) {
            // Mark As Dirty
            chunk->setDirty();
            level_renderer->dirty_chunks.push_back(chunk);
        }
    }
}

// Receive Rebuilt Chunks
static void GameRenderer_render_injection(GameRenderer_render_t original, GameRenderer *self, const float param_1) {
    // Receive Built Chunks
    static std::vector<void *> data;
    _receive_rebuilt_chunks(data);

    // Render Rebuilt Chunks
    Minecraft *minecraft = self->minecraft;
    LevelRenderer *level_renderer = minecraft->level_renderer;
    for (const void *msg : data) {
        // Check If Chunk Is Valid
        const rebuilt_chunk_data *chunk_data = (const rebuilt_chunk_data *) msg;
        bool valid = false;
        if (minecraft->isLevelGenerated() && level_renderer && level_renderer->chunks) {
            for (int i = 0; i < level_renderer->chunks_length; i++) {
                if (level_renderer->chunks[i] == chunk_data->chunk) {
                    valid = true;
                    break;
                }
            }
        }

        // Render
        if (valid) {
            render_rebuilt_chunk(chunk_data, level_renderer);
        }
        _free_rebuilt_chunk_data(chunk_data);
    }

    // Call Original Method
    original(self, param_1);
}

// Init
void init_chunk_rebuilding() {
    // Check Feature Flag
    if (!feature_has("Multithreaded Chunk Rebuilding", server_disabled)) {
        return;
    }

    // Start/Stop Thread
    overwrite_calls(LevelRenderer_setLevel, LevelRenderer_setLevel_injection);
    overwrite_calls(LevelRenderer_destructor_complete, LevelRenderer_destructor_injection);
    overwrite_calls(NinecraftApp_teardown, NinecraftApp_teardown_injection);

    // Build Chunks
    advanced_tesselator_enable();
    overwrite_calls(Chunk_rebuild, Chunk_rebuild_injection);
    overwrite_calls(GameRenderer_render, GameRenderer_render_injection);

    // Setup Biome Data
    _init_threaded_biome_source();
}