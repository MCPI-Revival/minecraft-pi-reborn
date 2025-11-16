#include <libreborn/log.h>

#include "thread.h"

// Input/Output
ThreadVector _chunks_to_rebuild;
ThreadVector _rebuilt_chunks;

// Build Chunk
static void build_chunk(chunk_rebuild_data *data, rebuilt_chunk_data *out) {
    // Prepare
    for (bool &empty : out->is_layer_empty) {
        empty = true;
    }
    for (const VertexArray<CustomVertexFlat> *&ptr : out->vertices) {
        ptr = nullptr;
    }
    out->is_empty = true;
    out->touched_sky = false;
    out->chunk = data->chunk;
    Tesselator &t = Tesselator::instance;

    // Get Chunk Information
    CachedLevelSource &source = data->source;
    source._cache();
    if (!source.should_render) {
        // Empty Chunk
        return;
    }
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
                        rendered |= tile_renderer->tesselateInWorld(tile, x, y, z);
                    }
                }
            }
        }
        // Save Layer
        if (started) {
            if (rendered) {
                out->vertices[layer] = advanced_tesselator_get().vertices_flat.copy();
                out->is_layer_empty[layer] = false;
                out->is_empty = false;
            }
            t.clear();
        }
    }

    // Clean Up
    ::operator delete(tile_renderer);
    out->touched_sky = source.touched_sky;
}

// Configure Rendering
void _configure_tesselator_for_chunk_rebuild(const bool enable) {
    CustomTesselator &t = advanced_tesselator_get();
    t.are_vertices_flat = enable;
    t.enable_real_quads = enable;
}

// Chunk Building Thread
struct chunk_rebuild_thread_data {
    int seed = 0;
};
static void *chunk_building_thread_func(void *arg) {
    // Configure
    _configure_tesselator_for_chunk_rebuild(true);
    const chunk_rebuild_thread_data *thread_data = (const chunk_rebuild_thread_data *) arg;
    _create_biome_source(thread_data->seed);
    delete thread_data;

    // Loop
    while (true) {
        std::vector<void *> data;
        _chunks_to_rebuild.receive(data, true);
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
            _rebuilt_chunks.add(chunk->chunk, out);
            delete chunk;
        }
    }
}

// Start/Stop
static pthread_t chunk_rebuild_thread;
static bool chunk_rebuild_thread_running = false;
void _stop_chunk_rebuild_thread() {
    // Kill Thread
    if (chunk_rebuild_thread_running) {
        // Signal Stop
        _chunks_to_rebuild.add(nullptr, nullptr);
        // Wait For Thread To Stop
        pthread_join(chunk_rebuild_thread, nullptr);
        chunk_rebuild_thread_running = false;
        // Free Remaining Messages
        std::vector<void *> data;
        _rebuilt_chunks.receive(data, false);
        for (const void *msg : data) {
            const rebuilt_chunk_data *chunk = (const rebuilt_chunk_data *) msg;
            _free_rebuilt_chunk_data(chunk);
        }
    }
}
void _start_chunk_rebuild_thread(Level *level) {
    if (chunk_rebuild_thread_running) {
        IMPOSSIBLE();
    }
    chunk_rebuild_thread_data *data = new chunk_rebuild_thread_data;
    data->seed = level->getSeed();
    pthread_create(&chunk_rebuild_thread, nullptr, chunk_building_thread_func, data);
    chunk_rebuild_thread_running = true;
}