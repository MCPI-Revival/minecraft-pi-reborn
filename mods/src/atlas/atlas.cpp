#include "internal.h"

#include <GLES/gl.h>

#include <libreborn/patch.h>
#include <libreborn/util/util.h>
#include <libreborn/env/env.h>

#include <symbols/Item.h>
#include <symbols/ItemRenderer.h>
#include <symbols/Tile.h>
#include <symbols/NinecraftApp.h>
#include <symbols/Textures.h>
#include <symbols/Texture.h>
#include <symbols/Tesselator.h>

#include <media-layer/core.h>

#include <mods/feature/feature.h>
#include <mods/misc/misc.h>
#include <mods/textures/textures.h>
#include <mods/atlas/atlas.h>
#include <mods/screenshot/screenshot.h>
#include <mods/init/init.h>

// Texture Name
static constexpr const char *atlas_texture_name = "gui/gui_blocks.png";

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
std::unordered_map<int, std::vector<std::pair<int, int>>> _tile_texture_to_atlas_pos;

// Render Atlas
static void generate_atlas(Minecraft *minecraft) {
    // Setup Offscreen Rendering
    media_begin_offscreen_render(atlas_texture_size, atlas_texture_size);

    // Setup OpenGL
    ((NinecraftApp *) minecraft)->initGLStates();
    media_glViewport(0, 0, atlas_texture_size, atlas_texture_size);
    media_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    media_glMatrixMode(GL_PROJECTION);
    media_glLoadIdentity();
    media_glOrthof(0, atlas_texture_size, atlas_texture_size, 0, 2000, 3000);
    media_glMatrixMode(GL_MODELVIEW);
    media_glLoadIdentity();
    media_glTranslatef(0, 0, -2000);
    media_glDisable(GL_DEPTH_TEST);

    // Render
    Textures *textures = minecraft->textures;
    _atlas_render(textures);

    // Copy Open Inventory Button
    _atlas_copy_inventory_button(textures, &minecraft->gui);

    // Dump
    if (is_env_set(MCPI_DUMP_ATLAS_ENV)) {
        screenshot_take(nullptr, "atlas-dumps");
    }

    // Get Line Size
    int line_size = atlas_texture_size * 4;
    int alignment;
    media_glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
    line_size = align_up(line_size, alignment);
    // Read Texture
    Texture texture = {};
    texture.width = atlas_texture_size;
    texture.height = atlas_texture_size;
    texture.data_size = atlas_texture_size * line_size;
    texture.has_alpha = true;
    texture.prevent_freeing_data = false;
    texture.field6_0x14 = 0;
    texture.field7_0x18 = -1;
    texture.data = new unsigned char[texture.data_size];
    media_glReadPixels(0, 0, atlas_texture_size, atlas_texture_size, GL_RGBA, GL_UNSIGNED_BYTE, texture.data);
    for (int y = 0; y < (texture.height / 2); y++) {
        for (int x = 0; x < (texture.width * 4); x++) {
            unsigned char &a = texture.data[(y * line_size) + x];
            unsigned char &b = texture.data[((texture.height - y - 1) * line_size) + x];
            std::swap(a, b);
        }
    }

    // Restore Old Context
    media_end_offscreen_render();

    // Upload Texture
    textures->assignTexture(atlas_texture_name, texture);
    DEBUG("Generated gui_blocks Atlas");
}
static void Minecraft_onGraphicsReset_injection(Minecraft_onGraphicsReset_t original, Minecraft *self) {
    // Call Original Method
    original(self);
    // Regenerate Atlas
    generate_atlas(self);
}

// Use New Atlas
static void ItemRenderer_renderGuiItem_two_injection(MCPI_UNUSED ItemRenderer_renderGuiItem_two_t original, MCPI_UNUSED Font *font, Textures *textures, const ItemInstance *item_instance_ptr, const float x, const float y, const float w, const float h, MCPI_UNUSED bool param_5) {
    // Replace "Carried" Items
    ItemInstance item_instance = *item_instance_ptr;
    if (item_instance.id == Tile::leaves->id) {
        item_instance.id = Tile::leaves_carried->id;
    } else if (item_instance.id == Tile::grass->id) {
        item_instance.id = Tile::grass_carried->id;
    }

    // Get Position In Atlas
    Item *item = Item::items[item_instance.id];
    if (!item) {
        return;
    }
    const int key = _atlas_get_key(item, item_instance.auxiliary);
    if (!_atlas_key_to_pos.contains(key)) {
        WARN("Skipping Item Not In gui_blocks Atlas: %i:%i", item_instance.id, item_instance.auxiliary);
        return;
    }
    const std::pair<int, int> &pos = _atlas_key_to_pos.at(key);

    // Convert To UV
    constexpr float scale = float(atlas_texture_size) / atlas_entry_size;
    const float u0 = float(pos.first) / scale;
    const float u1 = float(pos.first + 1) / scale;
    const float v0 = float(pos.second) / scale;
    const float v1 = float(pos.second + 1) / scale;

    // Render
    textures->loadAndBindTexture(atlas_texture_name);
    Tesselator &t = Tesselator::instance;
    t.begin(GL_QUADS);
    t.colorABGR(item_instance.count > 0 ? 0xffffffff : 0x60ffffff);
    t.vertexUV(x, y + h, 0, u0, v1);
    t.vertexUV(x + w, y + h, 0, u1, v1);
    t.vertexUV(x + w, y, 0, u1, v0);
    t.vertexUV(x, y, 0, u0, v0);
    t.draw();
}

// Update Dynamic Textures In Atlas
void atlas_update_tile(Textures *textures, const int texture, const unsigned char *pixels) {
    // Update Texture
    if (!_tile_texture_to_atlas_pos.contains(texture)) {
        return;
    }
    for (const std::pair<int, int> &pos : _tile_texture_to_atlas_pos.at(texture)) {
        const uint32_t atlas_id = textures->loadAndBindTexture(atlas_texture_name);
        const Texture *atlas_data = textures->getTemporaryTextureData(atlas_id);
        constexpr int texture_size = 16;
        constexpr float fake_atlas_size = atlas_texture_size * (float(texture_size) / atlas_entry_size);
        media_glTexSubImage2D_with_scaling(atlas_data, pos.first * texture_size, pos.second * texture_size, texture_size, texture_size, fake_atlas_size, fake_atlas_size, pixels);
    }
}

// Init
void init_atlas() {
    // Generate Atlas
    if (feature_has("Regenerate \"gui_blocks\" Atlas", server_disabled)) {
        misc_run_on_init(generate_atlas);
        overwrite_calls(Minecraft_onGraphicsReset, Minecraft_onGraphicsReset_injection);
        overwrite_calls(ItemRenderer_renderGuiItem_two, ItemRenderer_renderGuiItem_two_injection);
        _atlas_init_special_cases();
    }
}
