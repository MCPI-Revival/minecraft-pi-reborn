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
int _atlas_get_key(Item *item, int data);
extern std::unordered_map<int, std::pair<int, int>> _atlas_key_to_pos;

// Render The Atlas Itself
struct Textures;
void _atlas_build();
void _atlas_render(Textures *textures);
extern thread_local bool _atlas_active;

// Special/Edges Cases
void _atlas_init_special_cases();
struct Gui;
void _atlas_copy_inventory_button(Textures *textures, Gui *gui);