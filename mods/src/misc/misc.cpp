#include <string>
#include <fstream>
#include <streambuf>

#include <libcore/libcore.h>

#include "../util/cxx11_util.h"

#include "../feature/feature.h"
#include "misc.h"

#include "../minecraft.h"

// Read Asset File
static cxx11_string AppPlatform_readAssetFile_injection(__attribute__((unused)) unsigned char *app_platform, std::string const& path) {
    std::string full_path("./data/");
    full_path.append(path);
    std::ifstream stream(full_path);
    std::string str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    return create_cxx11_string(str.c_str());
}

static void inventory_add_item(unsigned char *inventory, unsigned char *item, bool is_tile) {
    unsigned char *item_instance = (unsigned char *) ::operator new(ITEM_INSTANCE_SIZE);
    item_instance = (*(is_tile ? ItemInstance_constructor_tile : ItemInstance_constructor_item))(item_instance, item);
    (*FillingContainer_addItem)(inventory, item_instance);
}

static int32_t Inventory_setupDefault_FillingContainer_addItem_call_injection(unsigned char *filling_container, unsigned char *item_instance) {
    // Call Original
    int32_t ret = (*FillingContainer_addItem)(filling_container, item_instance);

    // Add Items
    inventory_add_item(filling_container, *Item_flintAndSteel, false);
    inventory_add_item(filling_container, *Item_snowball, false);
    inventory_add_item(filling_container, *Item_egg, false);
    inventory_add_item(filling_container, *Item_shears, false);
    for (int i = 0; i < 15; i++) {
        unsigned char *item_instance = (unsigned char *) ::operator new(0xc);
        item_instance = (*ItemInstance_constructor_item_extra)(item_instance, *Item_dye_powder, 1, i);
        (*FillingContainer_addItem)(filling_container, item_instance);
    }
    inventory_add_item(filling_container, *Item_camera, false);
    // Add Tiles
    inventory_add_item(filling_container, *Tile_water, true);
    inventory_add_item(filling_container, *Tile_lava, true);
    inventory_add_item(filling_container, *Tile_calmWater, true);
    inventory_add_item(filling_container, *Tile_calmLava, true);
    inventory_add_item(filling_container, *Tile_glowingObsidian, true);
    inventory_add_item(filling_container, *Tile_web, true);
    inventory_add_item(filling_container, *Tile_topSnow, true);
    inventory_add_item(filling_container, *Tile_ice, true);
    inventory_add_item(filling_container, *Tile_invisible_bedrock, true);

    return ret;
}

// Render Selected Item Text
static void Gui_renderChatMessages_injection(unsigned char *gui, int32_t param_1, uint32_t param_2, bool param_3, unsigned char *font) {
    // Call Original Method
    (*Gui_renderChatMessages)(gui, param_1, param_2, param_3, font);
    // Calculate Selected Item Text Scale
    unsigned char *minecraft = *(unsigned char **) (gui + 0x9f4);
    int32_t screen_width = *(int32_t *) (minecraft + 0x20);
    float scale = ((float) screen_width) * *InvGuiScale;
    // Render Selected Item Text
    (*Gui_renderOnSelectItemNameText)(gui, (int32_t) scale, font, param_1 - 0x13);
}
// Reset Selected Item Text Timer On Slot Select
static bool reset_selected_item_text_timer = false;
static void Gui_tick_injection(unsigned char *gui) {
    // Call Original Method
    (*Gui_tick)(gui);
    // Handle Reset
    float *selected_item_text_timer = (float *) (gui + 0x9fc);
    if (reset_selected_item_text_timer) {
        // Reset
        *selected_item_text_timer = 0;
        reset_selected_item_text_timer = false;
    }
}
// Trigger Reset Selected Item Text Timer On Slot Select
static void Inventory_selectSlot_injection(unsigned char *inventory, int32_t slot) {
    // Call Original Method
    (*Inventory_selectSlot)(inventory, slot);
    // Trigger Reset Selected Item Text Timer
    reset_selected_item_text_timer = true;
}

void init_misc_cpp() {
    // Implement AppPlatform::readAssetFile So Translations Work
    overwrite((void *) AppPlatform_readAssetFile, (void *) AppPlatform_readAssetFile_injection);

    if (feature_has("Expand Creative Inventory")) {
        // Add Extra Items To Creative Inventory (Only Replace Specific Function Call)
        overwrite_call((void *) 0x8e0fc, (void *) Inventory_setupDefault_FillingContainer_addItem_call_injection);
    }

    // Fix Selected Item Text
    overwrite_calls((void *) Gui_renderChatMessages, (void *) Gui_renderChatMessages_injection);
    overwrite_calls((void *) Gui_tick, (void *) Gui_tick_injection);
    overwrite_calls((void *) Inventory_selectSlot, (void *) Inventory_selectSlot_injection);
}