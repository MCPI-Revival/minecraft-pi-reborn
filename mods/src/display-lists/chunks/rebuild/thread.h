#pragma once

#include <mods/tesselator/tesselator.h>

#include "CachedLevelSource.h"
#include "ThreadVector.h"

// Messages Sent To Rebuild Thread
struct chunk_rebuild_data {
    int seed = 0;
    Chunk *chunk = nullptr;
    CachedLevelSource source;
    int x0 = 0;
    int y0 = 0;
    int z0 = 0;
    int x1 = 0;
    int y1 = 0;
    int z1 = 0;
};
MCPI_INTERNAL extern ThreadVector _chunks_to_rebuild;

// Messages Received From Rebuild Thread
static constexpr int num_layers = 3;
struct rebuilt_chunk_data {
    Chunk *chunk = nullptr;
    struct empty {
        empty();
        bool value = true;
        bool layers[num_layers] = {};
        void apply(Chunk *out) const;
        void set(int layer);
    } is_empty;
    bool touched_sky = false;
    int old_sky_darken = 0;
    const VertexArray<CustomVertexFlat> *vertices[num_layers] = {};
};
MCPI_INTERNAL extern ThreadVector _rebuilt_chunks;
MCPI_INTERNAL void _receive_rebuilt_chunks(std::vector<void *> &data);
MCPI_INTERNAL void _free_rebuilt_chunk_data(const rebuilt_chunk_data *chunk);

// Configure Rendering
MCPI_INTERNAL void _configure_tesselator_for_chunk_rebuild(bool enable);

// Biome Data
MCPI_INTERNAL void _init_threaded_biome_source();
MCPI_INTERNAL void _create_biome_source(int seed);
MCPI_INTERNAL void _free_biome_source();

// Tile Rendering
MCPI_INTERNAL bool _render_tile(TileRenderer *tile_renderer, Tile *tile, int x, int y, int z);

// Start/Stop
MCPI_INTERNAL void _stop_chunk_rebuild_thread();
MCPI_INTERNAL void _start_chunk_rebuild_thread(Level *level);
