#pragma once

#include <mods/tesselator/tesselator.h>

#include "../CachedLevelSource.h"

// Messages Sent To Rebuild Thread
struct Chunk;
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
MCPI_INTERNAL void _free_rebuilt_chunk_data(const rebuilt_chunk_data *chunk);