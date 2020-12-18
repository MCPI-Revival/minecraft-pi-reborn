#include <GLES/gl.h>

#include <libcore/libcore.h>

#include "../feature/feature.h"
#include "../init/init.h"

#include "../minecraft.h"

static void Minecraft_tick_injection(unsigned char *minecraft, int32_t param_1, int32_t param_2) {
    // Call Original Method
    (*Minecraft_tick)(minecraft, param_1, param_2);

    // Tick Dynamic Textures
    unsigned char *textures = *(unsigned char **) (minecraft + Minecraft_textures_property_offset);
    if (textures != NULL) {
        (*Textures_tick)(textures, true);
    }
}

// Fix Grass And Leaves Inventory Rendering When The gui_blocks Atlas Is Disabled
static float ItemRenderer_renderGuiItemCorrect_injection(unsigned char *font, unsigned char *textures, unsigned char *item_instance, int32_t param_1, int32_t param_2) {
    int32_t leaves_id = *(int32_t *) (*Tile_leaves + Tile_id_property_offset);
    int32_t grass_id = *(int32_t *) (*Tile_grass + Tile_id_property_offset);
    // Replace Rendered Item With Carried Variant
    unsigned char *carried_item_instance = NULL;
    if (item_instance != NULL) {
        int32_t id = *(int32_t *) (item_instance + ItemInstance_id_property_offset);
        int32_t count = *(int32_t *) (item_instance + ItemInstance_count_property_offset);
        int32_t auxilary = *(int32_t *) (item_instance + ItemInstance_auxilary_property_offset);
        if (id == leaves_id) {
            carried_item_instance = (unsigned char *) ::operator new(ITEM_INSTANCE_SIZE);
            (*ItemInstance_constructor_title_extra)(carried_item_instance, *Tile_leaves_carried, count, auxilary);
        } else if (id == grass_id) {
            carried_item_instance = (unsigned char *) ::operator new(ITEM_INSTANCE_SIZE);
            (*ItemInstance_constructor_title_extra)(carried_item_instance, *Tile_grass_carried, count, auxilary);
        }
    }
    // Fix Toolbar Rendering
    GLboolean depth_test_was_enabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    // Call Original Method
    float ret = (*ItemRenderer_renderGuiItemCorrect)(font, textures, carried_item_instance != NULL ? carried_item_instance : item_instance, param_1, param_2);
    // Revert GL State Changes
    if (depth_test_was_enabled) {
        glEnable(GL_DEPTH_TEST);
    }
    // Free Carried Item Instance Variant
    if (carried_item_instance != NULL) {
        ::operator delete(carried_item_instance);
    }
    // Return
    return ret;
}

void init_textures() {
    if (feature_has("Animated Water")) {
        // Tick Dynamic Textures (Animated Water)
        overwrite_calls((void *) Minecraft_tick, (void *) Minecraft_tick_injection);
    }

    if (feature_has("Disable gui_blocks Atlas")) {
        // Disable gui_blocks Atlas Which Contains Pre-Rendered Textures For Blocks In The Inventory
        unsigned char disable_gui_blocks_atlas_patch[4] = {0x00, 0xf0, 0x20, 0xe3};
        patch((void *) 0x63c2c, disable_gui_blocks_atlas_patch);
        // Fix Grass And Leaves Inventory Rendering When The gui_blocks Atlas Is Disabled
        overwrite_calls((void *) ItemRenderer_renderGuiItemCorrect, (void *) ItemRenderer_renderGuiItemCorrect_injection);
    }
}