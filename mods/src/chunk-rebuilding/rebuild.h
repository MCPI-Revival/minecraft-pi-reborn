#pragma once

// Configure Rendering
void _configure_tesselator_for_chunk_rebuild(bool enable);

// Biome Data
void _init_threaded_biome_source();
void _create_biome_source(int seed);
void _free_biome_source();

// Tile Rendering
struct TileRenderer;
struct Tile;
bool _render_tile_safely(TileRenderer *tile_renderer, Tile *tile, int x, int y, int z);