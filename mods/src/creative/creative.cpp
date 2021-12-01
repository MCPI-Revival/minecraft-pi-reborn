#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "../init/init.h"
#include "../feature/feature.h"
#include "creative.h"

// Add Item To Inventory
static void inventory_add_item(unsigned char *inventory, unsigned char *item, bool is_tile) {
    ItemInstance *item_instance = new ItemInstance;
    ALLOC_CHECK(item_instance);
    item_instance = (*(is_tile ? ItemInstance_constructor_tile : ItemInstance_constructor_item))(item_instance, item);
    (*FillingContainer_addItem)(inventory, item_instance);
}

// Expand Creative Inventory
static int32_t Inventory_setupDefault_FillingContainer_addItem_call_injection(unsigned char *filling_container, ItemInstance *item_instance) {
    // Call Original
    int32_t ret = (*FillingContainer_addItem)(filling_container, item_instance);

    // Add Items
    inventory_add_item(filling_container, *Item_flintAndSteel, false);
    inventory_add_item(filling_container, *Item_snowball, false);
    inventory_add_item(filling_container, *Item_egg, false);
    inventory_add_item(filling_container, *Item_shears, false);
    // Dyes
    for (int i = 0; i < 16; i++) {
        if (i == 15) {
            // Bonemeal Is Already In The Creative Inventory
            continue;
        }
        ItemInstance *new_item_instance = new ItemInstance;
        ALLOC_CHECK(new_item_instance);
        new_item_instance = (*ItemInstance_constructor_item_extra)(new_item_instance, *Item_dye_powder, 1, i);
        (*FillingContainer_addItem)(filling_container, new_item_instance);
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
    inventory_add_item(filling_container, *Tile_bedrock, true);
    inventory_add_item(filling_container, *Tile_info_updateGame1, true);
    inventory_add_item(filling_container, *Tile_info_updateGame2, true);

    return ret;
}

// Check Restriction Status
static int is_restricted = 1;
int creative_is_restricted() {
    return is_restricted;
}

// Init
void init_creative() {
    // Add Extra Items To Creative Inventory (Only Replace Specific Function Call)
    if (feature_has("Expand Creative Inventory", 0)) {
        overwrite_call((void *) 0x8e0fc, (void *) Inventory_setupDefault_FillingContainer_addItem_call_injection);
    }

    // Remove Creative Restrictions (Opening Chests, Crafting, Etc)
    if (feature_has("Remove Creative Mode Restrictions", 0)) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        // Remove Restrictions
        patch((void *) 0x43ee8, nop_patch);
        patch((void *) 0x43f3c, nop_patch);
        patch((void *) 0x43f8c, nop_patch);
        patch((void *) 0x43fd8, nop_patch);
        patch((void *) 0x99010, nop_patch);
        // Fix UI
        patch((void *) 0x341c0, nop_patch);
        patch((void *) 0x3adb4, nop_patch);
        patch((void *) 0x3b374, nop_patch);
        // Fix Inventory
        patch((void *) 0x8d080, nop_patch);
        patch((void *) 0x8d090, nop_patch);
        patch((void *) 0x91d48, nop_patch);
        patch((void *) 0x92098, nop_patch);
        unsigned char inv_creative_check_r3_patch[4] = {0x03, 0x00, 0x53, 0xe1}; // "cmp r3, r3"
        patch((void *) 0x923c0, inv_creative_check_r3_patch);
        patch((void *) 0x92828, nop_patch);
        unsigned char inv_creative_check_r1_patch[4] = {0x01, 0x00, 0x51, 0xe1}; // "cmp r1, r1"
        patch((void *) 0x9282c, inv_creative_check_r1_patch);
        // Display Slot Count
        patch((void *) 0x1e3f4, nop_patch);
        unsigned char slot_count_patch[4] = {0x18, 0x00, 0x00, 0xea}; // "b 0x27110"
        patch((void *) 0x270a8, slot_count_patch);
        patch((void *) 0x33954, nop_patch);
        // Maximize Creative Inventory Stack Size
        unsigned char maximize_stack_patch[4] = {0xff, 0xc0, 0xa0, 0xe3}; // "mov r12, 0xff"
        patch((void *) 0x8e104, maximize_stack_patch);
        // Disable Other Restrictions
        is_restricted = 0;
    }
}
