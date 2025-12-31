#include <libreborn/log.h>
#include <GLES/gl.h>

#include <symbols/Tile.h>
#include <symbols/TileRenderer.h>
#include <symbols/Item.h>
#include <symbols/ItemRenderer.h>

#include "internal.h"

// Check If An Item Is A Tile And Renders Without A Model ("Flat" Rendering)
static bool is_flat_tile(const int id) {
    if (id < 256) {
        Tile *tile = Tile::tiles[id];
        if (tile && !TileRenderer::canRender(tile->getRenderShape())) {
            return true;
        }
    }
    return false;
}

// Render The Atlas Itself
void _atlas_render(Textures *textures) {
    int x = 0;
    int y = 0;
    // Loop Over All Possible IDs
    constexpr int id_count = int(sizeof(Item::items) / sizeof(Item *));
    for (int id = 0; id < id_count; id++) {
        Item *item = Item::items[id];
        if (!item) {
            // Invalid ID
            continue;
        }

        // Count Unique Textures
        constexpr int amount_of_data_values_to_check = 512;
        std::unordered_map<int, int> key_to_data;
        for (int data = amount_of_data_values_to_check - 1; data >= 0; data--) {
            int key = _atlas_get_key(item, data);
            key_to_data[key] = data;
        }

        // Loop Over All Data Values With Unique Textures
        for (const std::pair<int, int> info : key_to_data) {
            const int key = info.first;
            const int data = info.second;

            // Check Remaining Space (Leave Last Slot Empty)
            constexpr int last_entry_pos = atlas_size_entries - 1;
            if (x == last_entry_pos && y == last_entry_pos) {
                WARN("Out Of gui_blocks Atlas Space!");
                return;
            }

            // Position
            media_glPushMatrix();
            media_glTranslatef(atlas_entry_size * x, atlas_entry_size * y, 0);
            constexpr float scale = atlas_entry_size / 16.0f;
            media_glScalef(scale, scale, 1);

            // Render
            ItemInstance obj = {
                .count = 1,
                .id = id,
                .auxiliary = data
            };
            ItemRenderer::renderGuiItemCorrect(nullptr, textures, &obj, 0, 0);
            media_glPopMatrix();

            // Store
            const std::pair atlas_pos = {x, y};
            _atlas_key_to_pos.insert({key, atlas_pos});
            if (is_flat_tile(id)) {
                int icon = item->getIcon(data);
                _tile_texture_to_atlas_pos[icon].push_back(atlas_pos);
            }

            // Advance To Next Slot
            x++;
            if (x >= atlas_size_entries) {
                x = 0;
                y++;
            }
        }
    }
}