#include <GLES/gl.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>

// Fix Grass And Leaves Inventory Rendering When The gui_blocks Atlas Is Disabled
static void ItemRenderer_renderGuiItemCorrect_injection(Font *font, Textures *textures, ItemInstance *item_instance, int32_t param_1, int32_t param_2) {
    int32_t leaves_id = Tile_leaves->id;
    int32_t grass_id = Tile_grass->id;
    // Replace Rendered Item With Carried Variant
    ItemInstance carried_item_instance;
    bool use_carried = false;
    if (item_instance != NULL) {
        if (item_instance->id == leaves_id) {
            ItemInstance_constructor_tile_extra(&carried_item_instance, Tile_leaves_carried, item_instance->count, item_instance->auxiliary);
            use_carried = true;
        } else if (item_instance->id == grass_id) {
            ItemInstance_constructor_tile_extra(&carried_item_instance, Tile_grass_carried, item_instance->count, item_instance->auxiliary);
            use_carried = true;
        }
    }

    // Fix Toolbar Rendering
    GLboolean depth_test_was_enabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);

    // Call Original Method
    ItemRenderer_renderGuiItemCorrect(font, textures, use_carried ? &carried_item_instance : item_instance, param_1, param_2);

    // Revert GL State Changes
    if (depth_test_was_enabled) {
        glEnable(GL_DEPTH_TEST);
    }
}

// Fix Translucent Preview Items In Furnace UI Being Fully Opaque When The gui_blocks Atlas Is Disabled
static int item_color_fix_mode = 0;
#define POTENTIAL_FURNACE_ITEM_TRANSPARENCY 0x33
#define INVALID_FURNACE_ITEM_MULTIPLIER 0.25f
static void Tesselator_color_injection(Tesselator *tesselator, int32_t r, int32_t g, int32_t b, int32_t a) {
    // Fix Furnace UI
    if (item_color_fix_mode != 0) {
        // Force Translucent
        if (item_color_fix_mode == 1) {
            a = POTENTIAL_FURNACE_ITEM_TRANSPARENCY;
        } else {
            static double multiplier = INVALID_FURNACE_ITEM_MULTIPLIER;
            b *= multiplier;
            g *= multiplier;
            r *= multiplier;
        }
    }

    // Call Original Method
    Tesselator_color(tesselator, r, g, b, a);
}
static void Tesselator_begin_injection(Tesselator *tesselator, int32_t mode) {
    // Call Original Method
    Tesselator_begin(tesselator, mode);

    // Fix Furnace UI
    if (item_color_fix_mode != 0) {
        // Implict Translucent
        Tesselator_color_injection(tesselator, 0xff, 0xff, 0xff, 0xff);
    }
}
static void InventoryPane_renderBatch_Tesselator_color_injection(Tesselator *tesselator, int32_t r, int32_t g, int32_t b) {
    // Call Original Method
    Tesselator_color(tesselator, r, g, b, 0xff);

    // Enable Item Color Fix
    item_color_fix_mode = 2;
}
static void ItemRenderer_renderGuiItem_two_injection(Font *font, Textures *textures, ItemInstance *item_instance, float param_1, float param_2, float param_3, float param_4, bool param_5) {
    // Call Original Method
    ItemRenderer_renderGuiItem_two(font, textures, item_instance, param_1, param_2, param_3, param_4, param_5);

    // Disable Item Color Fix
    item_color_fix_mode = 0;
}
static void FurnaceScreen_render_ItemRenderer_renderGuiItem_one_injection(Font *font, Textures *textures, ItemInstance *item_instance, float param_1, float param_2, bool param_3) {
    // Enable Item Color Fix
    item_color_fix_mode = 1;

    // Call Original Method
    ItemRenderer_renderGuiItem_one(font, textures, item_instance, param_1, param_2, param_3);
}

// Init
void init_atlas() {
    // Add Better NULL-Check (And More UI Fixes When The gui_blocks Atlas Is Disabled)
    overwrite_calls((void *) ItemRenderer_renderGuiItem_two, (void *) ItemRenderer_renderGuiItem_two_injection);

    // Disable The gui_blocks Atlas Which Contains Pre-Rendered Textures For Blocks In The Inventory
    if (feature_has("Disable \"gui_blocks\" Atlas", server_disabled)) {
        unsigned char disable_gui_blocks_atlas_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x63c2c, disable_gui_blocks_atlas_patch);
        // Fix Grass And Leaves Inventory Rendering When The gui_blocks Atlas Is Disabled
        overwrite_calls((void *) ItemRenderer_renderGuiItemCorrect, (void *) ItemRenderer_renderGuiItemCorrect_injection);
        // Fix Furnace UI
        overwrite_calls((void *) Tesselator_begin, (void *) Tesselator_begin_injection);
        overwrite_calls((void *) Tesselator_color, (void *) Tesselator_color_injection);
        overwrite_call((void *) 0x32324, (void *) FurnaceScreen_render_ItemRenderer_renderGuiItem_one_injection);
        overwrite_call((void *) 0x1e21c, (void *) InventoryPane_renderBatch_Tesselator_color_injection);
    }
}
