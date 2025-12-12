#include <libreborn/patch.h>

#include <symbols/LevelRenderer.h>
#include <symbols/NinecraftApp.h>
#include <symbols/Chunk.h>
#include <symbols/Level.h>
#include <symbols/Tesselator.h>

#include <GLES/gl.h>

#include <mods/tesselator/tesselator.h>

#include "../../internal.h"
#include "thread.h"

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
    _chunks_to_rebuild.add(self, data);
}

// Receive Rebuilt Chunks
static void render_rebuilt_chunk(const rebuilt_chunk_data *chunk_data) {
    // Render Chunk
    Chunk *chunk = chunk_data->chunk;
    chunk->is_empty = chunk_data->is_empty;
    chunk->touched_sky = chunk_data->touched_sky;
    chunk->built = true;
    for (int layer = 0; layer < num_layers; layer++) {
        // Render Chunk Layer
        const bool is_empty = chunk_data->is_layer_empty[layer];
        chunk->is_layer_empty[layer] = is_empty;
        if (!is_empty) {
            // Prepare To Draw
            media_glNewList(chunk->display_lists + layer, GL_COMPILE);
            _configure_tesselator_for_chunk_rebuild(true);
            Tesselator &t = Tesselator::instance;
            t.begin(GL_QUADS);

            // Set Vertices
            advanced_tesselator_get().vertices_flat.receive(chunk_data->vertices[layer]);

            // Enable Colors And Texturing
            t.color(0, 0, 0, 0);
            t.tex(0, 0);

            // Draw
            t.draw();

            // Finish Display List
            media_glEndList();
            _configure_tesselator_for_chunk_rebuild(false);
        }
    }
}
static void LevelRenderer_tick_injection(LevelRenderer_tick_t original, LevelRenderer *self) {
    // Call Original Method
    original(self);

    // Receive Built Chunks
    static std::vector<void *> data;
    _rebuilt_chunks.receive(data, false);
    for (const void *msg : data) {
        // Check If Chunk Is Valid
        const rebuilt_chunk_data *chunk_data = (const rebuilt_chunk_data *) msg;
        bool valid = false;
        for (int i = 0; i < self->chunks_length; i++) {
            if (self->chunks[i] == chunk_data->chunk) {
                valid = true;
                break;
            }
        }

        // Render
        if (valid) {
            render_rebuilt_chunk(chunk_data);
        }
        _free_rebuilt_chunk_data(chunk_data);
    }
}

// Free Rebuilt Chunk Data
void _free_rebuilt_chunk_data(const rebuilt_chunk_data *chunk) {
    for (const VertexArray<CustomVertexFlat> *ptr : chunk->vertices) {
        delete ptr;
    }
    delete chunk;
}

// Init
void _init_display_lists_chunks_rebuild() {
    // Start/Stop Thread
    overwrite_calls(LevelRenderer_setLevel, LevelRenderer_setLevel_injection);
    overwrite_calls(LevelRenderer_destructor_complete, LevelRenderer_destructor_injection);
    overwrite_calls(NinecraftApp_teardown, NinecraftApp_teardown_injection);

    // Build Chunks
    advanced_tesselator_enable();
    overwrite_calls(Chunk_rebuild, Chunk_rebuild_injection);
    overwrite_calls(LevelRenderer_tick, LevelRenderer_tick_injection);

    // Setup Biome Data
    _init_threaded_biome_source();
}