#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "../init/init.h"
#include "../feature/feature.h"
#include "creative.h"

#ifndef MCPI_SERVER_MODE
// Add Item To Inventory
static void inventory_add_item(unsigned char *inventory, unsigned char *item, bool is_tile) {
    ItemInstance *item_instance = new ItemInstance;
    ALLOC_CHECK(item_instance);
    item_instance = (*(is_tile ? ItemInstance_constructor_tile : ItemInstance_constructor_item))(item_instance, item);
    (*FillingContainer_addItem)(inventory, item_instance);
}

// Expand Creative Inventory
static int32_t Inventory_setupDefault_FillingContainer_addItem_call_injection(unsigned char *filling_container, ItemInstance *item_instance) {
    // Call Original Method
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
    // Nether Reactor
    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            // Default Is Already In The Creative Inventory
            continue;
        }
        ItemInstance *new_item_instance = new ItemInstance;
        ALLOC_CHECK(new_item_instance);
        new_item_instance = (*ItemInstance_constructor_tile_extra)(new_item_instance, *Tile_netherReactor, 1, i);
        (*FillingContainer_addItem)(filling_container, new_item_instance);
    }
    // Tall Grass
    for (int i = 0; i < 4; i++) {
        if (i == 2) {
            // Identical To Previous Auxiliary Value
            continue;
        }
        ItemInstance *new_item_instance = new ItemInstance;
        ALLOC_CHECK(new_item_instance);
        new_item_instance = (*ItemInstance_constructor_tile_extra)(new_item_instance, *Tile_tallgrass, 1, i);
        (*FillingContainer_addItem)(filling_container, new_item_instance);
    }
    // Smooth Stone Slab
    {
        ItemInstance *new_item_instance = new ItemInstance;
        ALLOC_CHECK(new_item_instance);
        new_item_instance = (*ItemInstance_constructor_tile_extra)(new_item_instance, *Tile_stoneSlab, 1, 6);
        (*FillingContainer_addItem)(filling_container, new_item_instance);
    }

    return ret;
}
#endif

// Hook Specific TileItem Constructor
static unsigned char *Tile_initTiles_TileItem_injection(unsigned char *tile_item, int32_t id) {
    // Call Original Method
    unsigned char *ret = (*TileItem)(tile_item, id);

    // Switch VTable
    *(unsigned char **) tile_item = AuxDataTileItem_vtable;
    // Configure Item
    *(bool *) (tile_item + Item_is_stacked_by_data_property_offset) = true;
    *(int32_t *) (tile_item + Item_max_damage_property_offset) = 0;
    *(unsigned char **) (tile_item + AuxDataTileItem_icon_tile_property_offset) = Tile_tiles[id + 0x100];

    // Return
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
    if (feature_has("Expand Creative Inventory", server_enabled)) {
#ifndef MCPI_SERVER_MODE
        overwrite_call((void *) 0x8e0fc, (void *) Inventory_setupDefault_FillingContainer_addItem_call_injection);
#endif

        // Use AuxDataTileItem by default instead of TileItem, so tiles in the Creative
        // Inventory can have arbitrary auxiliary values.
        {
            // Fix Size
            unsigned char size_patch[4] = {AUX_DATA_TILE_ITEM_SIZE, 0x00, 0xa0, 0xe3}; // "mov r0, #AUX_DATA_TILE_ITEM_SIZE"
            patch((void *) 0xc6f64, size_patch);
            // Hook Constructor
            overwrite_call((void *) 0xc6f74, (void *) Tile_initTiles_TileItem_injection);
        }
    }

    // Remove Creative Restrictions (Opening Chests, Crafting, Etc)
    if (feature_has("Remove Creative Mode Restrictions", server_disabled)) {
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
        patch((void *) 0x92830, nop_patch);
        // Display Slot Count
        patch((void *) 0x1e3f4, nop_patch);
        unsigned char slot_count_patch[4] = {0x18, 0x00, 0x00, 0xea}; // "b 0x27110"
        patch((void *) 0x270a8, slot_count_patch);
        patch((void *) 0x33954, nop_patch);
        // Maximize Creative Inventory Stack Size
        unsigned char maximize_stack_patch[4] = {0xff, 0xc0, 0xa0, 0xe3}; // "mov r12, 0xff"
        patch((void *) 0x8e104, maximize_stack_patch);
        // Allow Nether Reactor
        patch((void *) 0xc0290, nop_patch);
        // Disable Other Restrictions
        is_restricted = 0;
    }
}
