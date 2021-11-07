#include <GLES/gl.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "../feature/feature.h"
#include "../init/init.h"

// Fix Grass And Leaves Inventory Rendering When The gui_blocks Atlas Is Disabled
static float ItemRenderer_renderGuiItemCorrect_injection(unsigned char *font, unsigned char *textures, ItemInstance *item_instance, int32_t param_1, int32_t param_2) {
    int32_t leaves_id = *(int32_t *) (*Tile_leaves + Tile_id_property_offset);
    int32_t grass_id = *(int32_t *) (*Tile_grass + Tile_id_property_offset);
    // Replace Rendered Item With Carried Variant
    ItemInstance carried_item_instance;
    bool use_carried = false;
    if (item_instance != NULL) {
        if (item_instance->id == leaves_id) {
            (*ItemInstance_constructor_tile_extra)(&carried_item_instance, *Tile_leaves_carried, item_instance->count, item_instance->auxiliary);
            use_carried = true;
        } else if (item_instance->id == grass_id) {
            (*ItemInstance_constructor_tile_extra)(&carried_item_instance, *Tile_grass_carried, item_instance->count, item_instance->auxiliary);
            use_carried = true;
        }
    }

    // Fix Toolbar Rendering
    GLboolean depth_test_was_enabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);

    // Call Original Method
    float ret = (*ItemRenderer_renderGuiItemCorrect)(font, textures, use_carried ? &carried_item_instance : item_instance, param_1, param_2);

    // Revert GL State Changes
    if (depth_test_was_enabled) {
        glEnable(GL_DEPTH_TEST);
    }

    // Return
    return ret;
}

// Fix Translucent Preview Items In Furnace UI Being Fully Opaque When The gui_blocks Atlas Is Disabled
static bool use_furnace_fix = false;
#define FURNACE_ITEM_TRANSPARENCY 0x33
static void Tesselator_colorABGR_injection(unsigned char *tesselator, int32_t color) {
    // Fix Furnace UI
    if (use_furnace_fix) {
        // Force Translucent
        int32_t a = FURNACE_ITEM_TRANSPARENCY;
        int32_t b = (color & 0x00ff0000) >> 16;
        int32_t g = (color & 0x0000ff00) >> 8;
        int32_t r = (color & 0x000000ff) >> 0;
        // New Color
        color = r | (g << 8) | (b << 16) | (a << 24);
    }

    // Call Original Method
    (*Tesselator_colorABGR)(tesselator, color);
}
static void Tesselator_begin_injection(unsigned char *tesselator, int32_t mode) {
    // Call Original Method
    (*Tesselator_begin)(tesselator, mode);

    // Fix Furnace UI
    if (use_furnace_fix) {
        // Implict Translucent
        (*Tesselator_colorABGR_injection)(tesselator, 0xffffffff);
    }
}
static void Tesselator_color_injection(unsigned char *tesselator, int32_t r, int32_t g, int32_t b, int32_t a) {
    // Fix Furnace UI
    if (use_furnace_fix) {
        // Force Translucent
        a = FURNACE_ITEM_TRANSPARENCY;
    }

    // Call Original Method
    (*Tesselator_color)(tesselator, r, g, b, a);
}
static float FurnaceScreen_render_ItemRenderer_renderGuiItem_injection(unsigned char *font, unsigned char *textures, ItemInstance *item_instance, float param_1, float param_2, bool param_3) {
    // Enable Furnace UI Fix
    use_furnace_fix = true;

    // Call Original Method
    float ret = (*ItemRenderer_renderGuiItem)(font, textures, item_instance, param_1, param_2, param_3);

    // Disable Furnace UI Fix
    use_furnace_fix = false;

    // Return
    return ret;
}

// Init
void init_atlas() {
    // Disable The gui_blocks Atlas Which Contains Pre-Rendered Textures For Blocks In The Inventory
    if (feature_has("Disable \"gui_blocks\" Atlas", 0)) {
        unsigned char disable_gui_blocks_atlas_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x63c2c, disable_gui_blocks_atlas_patch);
        // Fix Grass And Leaves Inventory Rendering When The gui_blocks Atlas Is Disabled
        overwrite_calls((void *) ItemRenderer_renderGuiItemCorrect, (void *) ItemRenderer_renderGuiItemCorrect_injection);
        // Fix Furnace UI
        overwrite_calls((void *) Tesselator_colorABGR, (void *) Tesselator_colorABGR_injection);
        overwrite_calls((void *) Tesselator_begin, (void *) Tesselator_begin_injection);
        overwrite_calls((void *) Tesselator_color, (void *) Tesselator_color_injection);
        overwrite_call((void *) 0x32324, (void *) FurnaceScreen_render_ItemRenderer_renderGuiItem_injection);
    }
}
