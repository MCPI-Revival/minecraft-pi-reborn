#include <GLES/gl.h>

#include <libreborn/patch.h>
#include <libreborn/util.h>

#include <symbols/minecraft.h>

#include <media-layer/core.h>

#include <mods/feature/feature.h>
#include <mods/misc/misc.h>
#include <mods/textures/textures.h>
#include <mods/atlas/atlas.h>
#include <mods/init/init.h>

// Atlas Texture
constexpr int atlas_texture_size = 2048; // Must Be Power Of Two

// Atlas Dimensions
constexpr int atlas_entry_size = 48;
constexpr int atlas_size_entries = atlas_texture_size / atlas_entry_size;

// Render Atlas
static int get_atlas_key(Item *item, const int data) {
    const int id = item->id;
    const int icon = item->getIcon(data);
    return (id << 16) | icon;
}
static std::unordered_map<int, std::pair<int, int>> atlas_key_to_pos;
static std::unordered_map<int, std::vector<std::pair<int, int>>> tile_texture_to_atlas_pos;
static bool is_flat_tile(const int id) {
    // Check If An Item Is A Tile
    if (id < 256) {
        Tile *tile = Tile::tiles[id];
        // Check If It Renders Without A Model ("Flat" Rendering)
        if (tile && !TileRenderer::canRender(tile->getRenderShape())) {
            return true;
        }
    }
    return false;
}
static void render_atlas(Textures *textures) {
    int x = 0;
    int y = 0;
    // Loop Over All Possible IDs
    for (int id = 0; id < 512; id++) {
        Item *item = Item::items[id];
        if (!item) {
            // Invalid ID
            continue;
        }
        // Count Unique Textures
        constexpr int amount_of_data_values_to_check = 512;
        std::unordered_map<int, int> key_to_data;
        for (int data = amount_of_data_values_to_check - 1; data >= 0; data--) {
            int key = get_atlas_key(item, data);
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
            atlas_key_to_pos[key] = {x, y};
            if (is_flat_tile(id)) {
                int icon = item->getIcon(data);
                tile_texture_to_atlas_pos[icon].push_back(atlas_key_to_pos[key]);
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

    // Re-Upload Textures
    Textures *textures = Textures::allocate();
    textures->constructor(&minecraft->options, minecraft->platform());

    // Render
    render_atlas(textures);

    // Copy Open Inventory Button
    textures->loadAndBindTexture("gui/gui_blocks.png");
    constexpr int icon_width = 28;
    constexpr int icon_height = 8;
    minecraft->gui.blit(atlas_texture_size - icon_width, atlas_texture_size - icon_height, 242, 252, icon_width, icon_height, 14, 4);

    // Read Texture
    int line_size = atlas_texture_size * 4;
    {
        // Handle Alignment
        int alignment;
        media_glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
        // Round
        line_size = ALIGN_UP(line_size, alignment);
    }
    Texture texture;
    texture.width = atlas_texture_size;
    texture.height = atlas_texture_size;
    texture.field3_0xc = 0;
    texture.field4_0x10 = true;
    texture.field5_0x11 = false;
    texture.field6_0x14 = 0;
    texture.field7_0x18 = -1;
    texture.data = new unsigned char[atlas_texture_size * line_size];
    media_glReadPixels(0, 0, atlas_texture_size, atlas_texture_size, GL_RGBA, GL_UNSIGNED_BYTE, texture.data);
    for (int y = 0; y < (texture.height / 2); y++) {
        for (int x = 0; x < (texture.width * 4); x++) {
            unsigned char &a = texture.data[(y * line_size) + x];
            unsigned char &b = texture.data[((texture.height - y - 1) * line_size) + x];
            std::swap(a, b);
        }
    }

    // Restore Old Context
    textures->destructor();
    ::operator delete(textures);
    media_end_offscreen_render();

    // Upload Texture
    minecraft->textures->assignTexture("gui/gui_blocks.png", texture);
    DEBUG("Generated gui_blocks Atlas");
}

// Use New Atlas
static void ItemRenderer_renderGuiItem_two_injection(__attribute__((unused)) ItemRenderer_renderGuiItem_two_t original, __attribute__((unused)) Font *font, Textures *textures, const ItemInstance *item_instance_ptr, const float x, const float y, const float w, const float h, __attribute__((unused)) bool param_5) {
    // "Carried" Items
    ItemInstance item_instance = *item_instance_ptr;
    if (item_instance.id == Tile::leaves->id) {
        item_instance.id = Tile::leaves_carried->id;
    } else if (item_instance.id == Tile::grass->id) {
        item_instance.id = Tile::grass_carried->id;
    }
    // Get Position
    Item *item = Item::items[item_instance.id];
    if (!item) {
        return;
    }
    const int key = get_atlas_key(item, item_instance.auxiliary);
    if (!atlas_key_to_pos.contains(key)) {
        WARN("Skipping Item Not In gui_blocks Atlas: %i:%i", item_instance.id, item_instance.auxiliary);
        return;
    }
    const std::pair<int, int> &pos = atlas_key_to_pos[key];
    // Convert To UV
    constexpr float scale = float(atlas_texture_size) / atlas_entry_size;
    float u0 = float(pos.first) / scale;
    float u1 = float(pos.first + 1) / scale;
    float v0 = float(pos.second) / scale;
    float v1 = float(pos.second + 1) / scale;
    // Render
    textures->loadAndBindTexture("gui/gui_blocks.png");
    Tesselator &t = Tesselator::instance;
    t.begin(GL_QUADS);
    t.colorABGR(item_instance.count > 0 ? 0xffffffff : 0x60ffffff);
    t.vertexUV(x, y + h, 0, u0, v1);
    t.vertexUV(x + w, y + h, 0, u1, v1);
    t.vertexUV(x + w, y, 0, u1, v0);
    t.vertexUV(x, y, 0, u0, v0);
    t.draw();
}

// Fix Buggy Crop Textures
#define MAX_CROP_DATA 7
static int CropTile_getTexture2_injection(CropTile_getTexture2_t original, CropTile *self, const int face, int data) {
    if (data > MAX_CROP_DATA) {
        data = MAX_CROP_DATA;
    }
    return original(self, face, data);
}

// Fix Open Inventory Button
static void Gui_renderToolBar_GuiComponent_blit_injection(GuiComponent *self, int x_dest, int y_dest, __attribute__((unused)) const int x_src, __attribute__((unused)) const int y_src, const int width_dest, const int height_dest, const int width_src, const int height_src) {
    constexpr float size_scale = 2.0f / atlas_texture_size;
    constexpr float u1 = 1.0f;
    const float u0 = u1 - (float(width_src) * size_scale);
    constexpr float v1 = 1.0f;
    const float v0 = v1 - (float(height_src) * size_scale);
    Tesselator &t = Tesselator::instance;
    t.begin(GL_QUADS);
    t.vertexUV(x_dest, y_dest + height_dest, self->z, u0, v1);
    t.vertexUV(x_dest + width_dest, y_dest + height_dest, self->z, u1, v1);
    t.vertexUV(x_dest + width_dest, y_dest, self->z, u1, v0);
    t.vertexUV(x_dest, y_dest, self->z, u0, v0);
    t.draw();
}

// Update Dynamic Textures In Atlas
void atlas_update_tile(Textures *textures, const int texture, const unsigned char *pixels) {
    // Update Texture
    for (const std::pair<int, int> &pos : tile_texture_to_atlas_pos[texture]) {
        uint32_t atlas_id = textures->loadAndBindTexture("gui/gui_blocks.png");
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
        overwrite_calls(CropTile_getTexture2, CropTile_getTexture2_injection);
        overwrite_calls(ItemRenderer_renderGuiItem_two, ItemRenderer_renderGuiItem_two_injection);
        overwrite_call((void *) 0x26f50, (void *) Gui_renderToolBar_GuiComponent_blit_injection);
    }
}
