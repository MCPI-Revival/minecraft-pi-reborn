#include <libreborn/log.h>
#include <GLES/gl.h>

#include <symbols/Item.h>
#include <symbols/ItemRenderer.h>

#include <mods/common.h>

#include "internal.h"

// Atlas Information (Keys And Positions)
int _atlas_get_key(Item *item, const int data) {
    _atlas_active = true;
    const int id = item->id;
    const int icon = item->getIcon(data);
    const int key = (id << 16) | icon;
    _atlas_active = false;
    return key;
}
std::unordered_map<int, std::pair<int, int>> _atlas_key_to_pos;

// Track Atlas Entries
struct AtlasEntry {
    std::pair<int, int> pos;
    ItemInstance item = {};
};
static std::vector<AtlasEntry> entries;

// Build The Atlas
void _atlas_build() {
    // Clear Old Entries
    entries.clear();
    _atlas_key_to_pos.clear();

    // Loop Over All Possible IDs
    std::vector<ItemInstance> items;
    constexpr size_t id_count = countof(Item::items);
    for (size_t id = 0; id < id_count; id++) {
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

            // Store
            items.push_back({
                .count = key,
                .id = int(id),
                .auxiliary = data
            });
        }
    }

    // Add Items To Atlas
    int x = 0;
    int y = 0;
    for (const ItemInstance &item : items) {
        // Check Remaining Space (Leave Last Slot Empty)
        constexpr int last_entry_pos = atlas_size_entries - 1;
        if (x == last_entry_pos && y == last_entry_pos) {
            WARN("Out Of gui_blocks Atlas Space!");
            break;
        }

        // Store
        const std::pair atlas_pos = {x, y};
        const int key = item.count;
        _atlas_key_to_pos.insert({key, atlas_pos});
        entries.push_back({
            .pos = atlas_pos,
            .item = item
        });

        // Advance To Next Slot
        x++;
        if (x >= atlas_size_entries) {
            x = 0;
            y++;
        }
    }

    // Check
    if (entries.size() != _atlas_key_to_pos.size()) {
        IMPOSSIBLE();
    }
}

// Render The Atlas Itself
void _atlas_render(Textures *textures) {
    // Check
    if (entries.empty()) {
        IMPOSSIBLE();
    }

    // Render
    for (const AtlasEntry &entry : entries) {
        // Position
        media_glPushMatrix();
        media_glTranslatef(atlas_entry_size * entry.pos.first, atlas_entry_size * entry.pos.second, 0);
        constexpr float scale = atlas_entry_size / 16.0f;
        media_glScalef(scale, scale, 1);

        // Render
        media_glColor4f(1, 1, 1, 1);
        ItemRenderer::renderGuiItemCorrect(nullptr, textures, &entry.item, 0, 0);
        media_glColor4f(1, 1, 1, 1);
        media_glPopMatrix();
    }
}