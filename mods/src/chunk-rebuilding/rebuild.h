#pragma once

// Configure Rendering
MCPI_INTERNAL void _configure_tesselator_for_chunk_rebuild(bool enable);

// Biome Data
MCPI_INTERNAL void _init_threaded_biome_source();
MCPI_INTERNAL void _create_biome_source(int seed);
MCPI_INTERNAL void _free_biome_source();

// Tile Rendering
struct TileRenderer;
struct Tile;
MCPI_INTERNAL bool _render_tile_safely(TileRenderer *tile_renderer, Tile *tile, int x, int y, int z);