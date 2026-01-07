#include "worker.h"
#include "messages.h"
#include "../rebuild.h"

#include <malloc.h>

#include <symbols/Tesselator.h>
#include <symbols/TileRenderer.h>
#include <symbols/Tile.h>

// Build Chunk
static void build_chunk(chunk_rebuild_data *data, rebuilt_chunk_data *out) {
    // Get Tesselator
    Tesselator &t = Tesselator::instance;
    CustomTesselator &advanced_t = advanced_tesselator_get();

    // Get Chunk Information
    out->chunk = data->chunk;
    CachedLevelSource &source = data->source;
    source._cache();
    out->old_sky_darken = source.level_sky_darken;
    if (!source.should_render) {
        // Empty Chunk, Skip Tessellating
        return;
    }
    out->touched_sky = source.touched_sky;

    // Create Tile Renderer
    TileRenderer *tile_renderer = TileRenderer::allocate();
    tile_renderer->constructor(source.self);

    // Render
    bool should_render_layer[num_layers] = {true, false, false};
    for (int layer = 0; layer < num_layers; layer++) {
        if (!should_render_layer[layer]) {
            continue;
        }
        // Render Layer
        bool started = false;
        bool rendered = false;
        for (int y = data->y0; y < data->y1; y++) {
            for (int z = data->z0; z < data->z1; z++) {
                for (int x = data->x0; x < data->x1; x++) {
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
                            t.begin(GL_QUADS);
                        }
                        rendered |= _render_tile_safely(tile_renderer, tile, x, y, z);
                    }
                }
            }
        }
        // Save Layer
        if (started) {
            if (rendered) {
                out->vertices[layer] = advanced_t.vertices_flat.copy();
                out->is_empty.set(layer);
            }
            advanced_t.clear(true);
        }
    }

    // Clean Up
    ::operator delete(tile_renderer);
}

// Chunk Building Thread
void *RebuildWorker::run(void *arg) {
    // Configure
    RebuildWorker *self = (RebuildWorker *) arg;
    _configure_tesselator_for_chunk_rebuild(true);
    _create_biome_source(self->seed);

    // Loop
    while (true) {
        std::vector<void *> data;
        self->input.receive(data, true);
        for (void *msg : data) {
            // Check
            if (!msg) {
                // End Of Data
                _free_biome_source();
                return nullptr;
            }

            // Build Chunk
            chunk_rebuild_data *chunk = (chunk_rebuild_data *) msg;
            rebuilt_chunk_data *out = new rebuilt_chunk_data;
            build_chunk(chunk, out);

            // Send Result
            self->output.add(chunk->chunk, out);

            // Force Release Memory
            delete chunk;
            malloc_trim(0);
        }
    }
}