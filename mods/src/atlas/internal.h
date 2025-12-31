#pragma once

#include <unordered_map>
#include <vector>

// Atlas Texture Size
// Must Be Power Of Two
static constexpr int atlas_texture_size = 2048;

// Atlas Dimensions
static constexpr int atlas_entry_size = 48;
static constexpr int atlas_size_entries = atlas_texture_size / atlas_entry_size;

// Atlas Information (Keys And Positions)
struct Item;
MCPI_INTERNAL int _atlas_get_key(Item *item, int data);
MCPI_INTERNAL extern std::unordered_map<int, std::pair<int, int>> _atlas_key_to_pos;
MCPI_INTERNAL extern std::unordered_map<int, std::vector<std::pair<int, int>>> _tile_texture_to_atlas_pos;

// Render The Atlas Itself
struct Textures;
MCPI_INTERNAL void _atlas_render(Textures *textures);
MCPI_INTERNAL extern thread_local bool _atlas_active;

// Special/Edges Cases
MCPI_INTERNAL void _atlas_init_special_cases();
struct Gui;
MCPI_INTERNAL void _atlas_copy_inventory_button(Textures *textures, Gui *gui);