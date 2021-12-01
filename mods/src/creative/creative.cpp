#include <vector>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include <mods/misc/misc.h>
#include <mods/creative/creative.h>

#ifndef MCPI_SERVER_MODE
// Add Item To Inventory
static void inventory_add_item(unsigned char *inventory, unsigned char *item, bool is_tile) {
    ItemInstance *item_instance = new ItemInstance;
    ALLOC_CHECK(item_instance);
    item_instance = (*(is_tile ? ItemInstance_constructor_tile : ItemInstance_constructor_item))(item_instance, item);
    (*FillingContainer_addItem)(inventory, item_instance);
}

// Expand Creative Inventory
static void Inventory_setupDefault_FillingContainer_addItem_call_injection(unsigned char *filling_container) {
    // Add Items
    inventory_add_item(filling_container, *Item_sign, false);
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
}
#endif

// Store All Default TileItems
static std::vector<unsigned char *> &get_default_tile_items() {
    static std::vector<unsigned char *> tile_items;
    return tile_items;
}
// Hook Specific TileItem :;operator new
static unsigned char *Tile_initTiles_operator_new_injection(__attribute__((unused)) uint32_t size) {
    // Call Original Method
    unsigned char *ret = (unsigned char *) ::operator new(AUX_DATA_TILE_ITEM_SIZE);

    // Store
    get_default_tile_items().push_back(ret);

    // Return
    return ret;
}
// Modify All Default TileItems
static void Tile_initTiles_injection(__attribute__((unused)) unsigned char *null) {
    // Loop
    for (unsigned char *tile_item : get_default_tile_items()) {
        // Get ID
        int32_t id = *(int32_t *) (tile_item + Item_id_property_offset);
        // Switch VTable
        *(unsigned char **) tile_item = AuxDataTileItem_vtable;
        // Configure Item
        *(bool *) (tile_item + Item_is_stacked_by_data_property_offset) = true;
        *(int32_t *) (tile_item + Item_max_damage_property_offset) = 0;
        *(unsigned char **) (tile_item + AuxDataTileItem_icon_tile_property_offset) = Tile_tiles[id];
    }
    get_default_tile_items().clear();
}

// Check Restriction Status
static int is_restricted = 1;
int creative_is_restricted() {
    return is_restricted;
}

// Init
void init_creative() {
    // Add Extra Items To Creative Inventory (Only Replace Specific Function Call)
    if (feature_has("Expand Creative Mode Inventory", server_enabled)) {
#ifndef MCPI_SERVER_MODE
        misc_run_on_creative_inventory_setup(Inventory_setupDefault_FillingContainer_addItem_call_injection);
#endif

        // Use AuxDataTileItem by default instead of TileItem, so tiles in the Creative
        // Inventory can have arbitrary auxiliary values.
        {
            // Hook TileItem ::operator new To Store TileItems
            overwrite_call((void *) 0x1295a4, (void *) Tile_initTiles_operator_new_injection);
            // Modify Stored TileItems
            misc_run_on_internal_after_tiles_setup(Tile_initTiles_injection);
        }
    }

    // Remove Creative Mode Restrictions (Opening Chests, Crafting, Etc)
    if (feature_has("Remove Creative Mode Restrictions", server_enabled)) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        // Remove Restrictions
        patch((void *) 0x59e68, nop_patch);
        patch((void *) 0x59ebc, nop_patch);
        patch((void *) 0x59f10, nop_patch);
        unsigned char allow_eating_patch[4] = {0x02, 0x00, 0x00, 0xea}; // "b 0xddcbc"
        patch((void *) 0xddcac, allow_eating_patch);
        // Fix UI
        patch((void *) 0x4cb88, nop_patch);
        unsigned char fix_ui_patch[4] = {0x05, 0x00, 0x55, 0xe1}; // "cmp r5, r5"
        patch((void *) 0x4bf20, fix_ui_patch);
        // Fix Inventory
        patch((void *) 0xcce90, nop_patch);
        patch((void *) 0xd5548, nop_patch);
        unsigned char inv_creative_check_r3_patch[4] = {0x03, 0x00, 0x53, 0xe1}; // "cmp r3, r3"
        patch((void *) 0xd497c, inv_creative_check_r3_patch);
        unsigned char inv_creative_check_r5_patch[4] = {0x05, 0x00, 0x55, 0xe1}; // "cmp r5, r5"
        patch((void *) 0xd4d94, inv_creative_check_r5_patch);
        patch((void *) 0xd50ac, nop_patch);
        patch((void *) 0x8d080, nop_patch);
        patch((void *) 0x8d090, nop_patch);
        // Display Slot Count
        patch((void *) 0x23d4c, nop_patch);
        patch((void *) 0x2c570, nop_patch);
        patch((void *) 0x3eec0, nop_patch);
        // Maximize Creative Inventory Stack Size
        unsigned char maximize_stack_patch[4] = {0xff, 0x60, 0xa0, 0xe3}; // "mov r6, 0xff"
        patch((void *) 0xccf80, maximize_stack_patch);
        // Allow Nether Reactor
        unsigned char nether_reactor_patch[4] = {0x00, 0x00, 0xa0, 0xe3}; // "mov r0, #0x0"
        patch((void *) 0x12283c, nether_reactor_patch);
        // Disable Other Restrictions
        is_restricted = 0;
    }
}
