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
#include <mods/screenshot/screenshot.h>
#include <mods/init/init.h>

// Texture Name
static constexpr const char *atlas_texture_name = "gui/gui_blocks.png";

// Prepare To Render Atlas
static void reset_bound_texture(const Minecraft *minecraft) {
    Textures *textures = minecraft->textures;
    textures->current_texture = 0;
    media_glBindTexture(GL_TEXTURE_2D, textures->current_texture);
}
static void actually_generate_atlas(Minecraft *minecraft) {
    // Render
    reset_bound_texture(minecraft);
    Textures *textures = minecraft->textures;
    _atlas_render(textures);

    // Copy Open Inventory Button
    _atlas_copy_inventory_button(textures, &minecraft->gui);
    reset_bound_texture(minecraft);
}

// Render Atlas
static GLuint texture_id;
static GLuint list;
static void generate_atlas(Minecraft *minecraft, const bool dump) {
    // Setup Offscreen Rendering
    reset_bound_texture(minecraft);
    media_begin_offscreen_render(texture_id);

    // Setup OpenGL
    ((NinecraftApp *) minecraft)->initGLStates();
    media_glViewport(0, 0, atlas_texture_size, atlas_texture_size);
    media_glClearColor(0, 0, 0, 0);
    media_glClear(GL_COLOR_BUFFER_BIT);
    const std::vector<GLenum> matrix_modes = {GL_MODELVIEW, GL_PROJECTION};
    for (const GLenum mode : matrix_modes) {
        media_glMatrixMode(mode);
        media_glPushMatrix();
        media_glLoadIdentity();
    }
    media_glOrthof(0, atlas_texture_size, atlas_texture_size, 0, -1000, 1000);
    media_glDisable(GL_DEPTH_TEST);

    // Render
    media_glCallLists(1, GL_UNSIGNED_INT, &list);

    // Dump
    if (dump) {
        screenshot_take(nullptr, "atlas-dumps");
    }

    // Restore Old OpenGL State
    media_end_offscreen_render();
    for (const GLenum mode : matrix_modes) {
        media_glMatrixMode(mode);
        media_glPopMatrix();
    }
    media_glEnable(GL_DEPTH_TEST);
    media_glViewport(0, 0, minecraft->screen_width, minecraft->screen_height);
}

// Create Atlas Texture & Display List
static void prepare_atlas(Minecraft *minecraft) {
    // Get Line Size
    int line_size = atlas_texture_size * 4;
    int alignment;
    media_glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
    line_size = align_up(line_size, alignment);

    // Create Texture
    Texture texture = {};
    texture.width = atlas_texture_size;
    texture.height = atlas_texture_size;
    texture.data_size = atlas_texture_size * line_size;
    texture.has_alpha = true;
    texture.prevent_freeing_data = false;
    texture.field6_0x14 = 0;
    texture.field7_0x18 = -1;
    texture.data = nullptr;
    Textures *textures = minecraft->textures;
    texture_id = minecraft->textures->assignTexture(atlas_texture_name, texture);

    // Load Textures
    const std::vector<std::string> needed_textures = {
        "terrain.png",
        "gui/items.png"
    };
    for (const std::string &needed_texture : needed_textures) {
        textures->loadTexture(needed_texture, true);
    }

    // Create Display List
    list = media_glGenLists(1);
    media_glNewList(list, GL_COMPILE);
    actually_generate_atlas(minecraft);
    media_glEndList();
}

// Handle Events
static void handle_event(Minecraft *minecraft, const bool should_prepare, const bool can_dump) {
    if (should_prepare) {
        prepare_atlas(minecraft);
    }
    generate_atlas(minecraft, can_dump && is_env_set(MCPI_DUMP_ATLAS_ENV));
}
static void on_init(Minecraft *minecraft) {
    handle_event(minecraft, true, true);
}
static void on_tick(Minecraft *minecraft) {
    handle_event(minecraft, false, false);
}
static void Minecraft_onGraphicsReset_injection(Minecraft_onGraphicsReset_t original, Minecraft *self) {
    original(self);
    handle_event(self, true, false);
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
    const float v0 = 1.0f - (float(pos.second) / scale);
    const float v1 = 1.0f - (float(pos.second + 1) / scale);

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

// Init
void init_atlas() {
    // Generate Atlas
    if (feature_has("Regenerate \"gui_blocks\" Atlas", server_disabled)) {
        misc_run_on_init(on_init);
        overwrite_calls(Minecraft_onGraphicsReset, Minecraft_onGraphicsReset_injection);
        misc_run_on_tick(on_tick);
        overwrite_calls(ItemRenderer_renderGuiItem_two, ItemRenderer_renderGuiItem_two_injection);
        _atlas_init_special_cases();
    }
}
