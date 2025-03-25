#include <libreborn/patch.h>

#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include <mods/misc/misc.h>
#include <mods/creative/creative.h>

// Add Item To Inventory
static void inventory_add_item(FillingContainer *inventory, Item *item) {
    ItemInstance *item_instance = new ItemInstance;
    item_instance = item_instance->constructor_item(item);
    inventory->addItem(item_instance);
}
static void inventory_add_item(FillingContainer *inventory, Tile *item) {
    ItemInstance *item_instance = new ItemInstance;
    item_instance = item_instance->constructor_tile(item);
    inventory->addItem(item_instance);
}

// Expand Creative Inventory
static void Inventory_setupDefault_FillingContainer_addItem_call_injection(FillingContainer *filling_container) {
    // Add Items
    inventory_add_item(filling_container, Item::flintAndSteel);
    inventory_add_item(filling_container, Item::snowball);
    inventory_add_item(filling_container, Item::egg);
    inventory_add_item(filling_container, Item::shears);
    // Dyes
    for (int i = 0; i < 16; i++) {
        if (i == 15) {
            // Bonemeal Is Already In The Creative Inventory
            continue;
        }
        ItemInstance *new_item_instance = new ItemInstance;
        new_item_instance = new_item_instance->constructor_item_extra(Item::dye_powder, 1, i);
        filling_container->addItem(new_item_instance);
    }
    inventory_add_item(filling_container, Item::camera);
    // Add Tiles
    inventory_add_item(filling_container, Tile::water);
    inventory_add_item(filling_container, Tile::lava);
    inventory_add_item(filling_container, Tile::calmWater);
    inventory_add_item(filling_container, Tile::calmLava);
    inventory_add_item(filling_container, Tile::glowingObsidian);
    inventory_add_item(filling_container, Tile::web);
    inventory_add_item(filling_container, Tile::topSnow);
    inventory_add_item(filling_container, Tile::ice);
    inventory_add_item(filling_container, Tile::invisible_bedrock);
    inventory_add_item(filling_container, Tile::bedrock);
    inventory_add_item(filling_container, Tile::info_updateGame1);
    inventory_add_item(filling_container, Tile::info_updateGame2);
    // Nether Reactor
    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            // Default Is Already In The Creative Inventory
            continue;
        }
        ItemInstance *new_item_instance = new ItemInstance;
        new_item_instance = new_item_instance->constructor_tile_extra(Tile::netherReactor, 1, i);
        filling_container->addItem(new_item_instance);
    }
    // Tall Grass
    for (int i = 0; i < 4; i++) {
        if (i == 2) {
            // Identical To Previous Auxiliary Value
            continue;
        }
        ItemInstance *new_item_instance = new ItemInstance;
        new_item_instance = new_item_instance->constructor_tile_extra(Tile::tallgrass, 1, i);
        filling_container->addItem(new_item_instance);
    }
    // Smooth Stone Slab
    {
        ItemInstance *new_item_instance = new ItemInstance;
        new_item_instance = new_item_instance->constructor_tile_extra(Tile::stoneSlab, 1, 6);
        filling_container->addItem(new_item_instance);
    }
}

// Hook Specific TileItem Constructor
static TileItem *Tile_initTiles_TileItem_injection(TileItem *tile_item, int32_t id) {
    // Call Original Method
    tile_item->constructor(id);

    // Switch VTable
    tile_item->vtable = (TileItem_vtable *) AuxDataTileItem_vtable::base;
    // Configure Item
    tile_item->is_stacked_by_data = true;
    tile_item->max_damage = 0;
    ((AuxDataTileItem *) tile_item)->icon_tile = Tile::tiles[id + 0x100];

    // Return
    return tile_item;
}

// Check Restriction Status
static bool is_restricted = true;
bool creative_is_restricted() {
    return is_restricted;
}

// Allow Creative Players To Drop Items
static bool Gui_tickItemDrop_Minecraft_isCreativeMode_call_injection(__attribute__((unused)) Minecraft *minecraft) {
    return false;
}

// Maximize Creative Inventory Stack Size
static void Inventory_setupDefault_injection(Inventory_setupDefault_t original, Inventory *self) {
    // Call Original Method
    original(self);
    // Update Item Counts
    if (self->is_creative) {
        for (ItemInstance *item : self->items) {
            if (item) {
                item->count = item->getMaxStackSize();
            }
        }
    }
}

// Init
void init_creative() {
    // Add Extra Items To Creative Inventory (Only Replace Specific Function Call)
    if (feature_has("Expand Creative Mode Inventory", server_enabled)) {
        misc_run_on_creative_inventory_setup(Inventory_setupDefault_FillingContainer_addItem_call_injection);

        // Use AuxDataTileItem by default instead of TileItem, so tiles in the Creative
        // Inventory can have arbitrary auxiliary values.
        {
            // Fix Size
            unsigned char size_patch[4] = {sizeof(AuxDataTileItem), 0x00, 0xa0, 0xe3}; // "mov r0, #AUX_DATA_TILE_ITEM_SIZE"
            patch((void *) 0xc6f64, size_patch);
            // Hook Constructor
            overwrite_call((void *) 0xc6f74, TileItem_constructor, Tile_initTiles_TileItem_injection);
        }
    }

    // Remove Creative Mode Restrictions (Opening Chests, Crafting, Etc)
    unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    if (feature_has("Remove Creative Mode Restrictions", server_enabled)) {
        // Remove Restrictions
        patch((void *) 0x43ee8, nop_patch);
        patch((void *) 0x43f3c, nop_patch);
        patch((void *) 0x43f8c, nop_patch);
        patch((void *) 0x43fd8, nop_patch);
        patch((void *) 0x99010, nop_patch);
        // Allow Nether Reactor
        patch((void *) 0xc0290, nop_patch);
        // Item Dropping
        void *addr = (void *) 0x27800;
        const void *func = extract_from_bl_instruction((unsigned char *) addr);
        if (func == Minecraft_isCreativeMode->backup) {
            overwrite_call(addr, Minecraft_isCreativeMode, Gui_tickItemDrop_Minecraft_isCreativeMode_call_injection);
        } else {
            // Handled By input/misc.cpp
        }
        // Disable Other Restrictions
        is_restricted = false;
    }

    // Inventory Behavior
    if (feature_has("Force Survival Mode Inventory Behavior", server_enabled)) {
        patch((void *) 0x8d080, nop_patch); // Inventory::add
        patch((void *) 0x92828, nop_patch); // FillingContainer::add
        patch((void *) 0x91d48, nop_patch); // FillingContainer::hasResource
        patch((void *) 0x92098, nop_patch); // FillingContainer::removeResource(int)
        unsigned char inv_creative_check_r3_patch[4] = {0x03, 0x00, 0x53, 0xe1}; // "cmp r3, r3"
        patch((void *) 0x923c0, inv_creative_check_r3_patch); // FillingContainer::removeResource(ItemInstance const&, bool)
    }

    // "Craft" And "Armor" Buttons
    if (feature_has("Force Survival Mode Inventory UI", server_enabled)) {
        patch((void *) 0x341c0, nop_patch); // Add "Armor" Button To Classic Inventory
        unsigned char inv_creative_check_r5_patch[4] = {0x05, 0x00, 0x55, 0xe1}; // "cmp r5, r5"
        patch((void *) 0x3adb0, inv_creative_check_r5_patch); // Reposition "Select blocks" In Touch Inventory
        patch((void *) 0x3b374, nop_patch); // Add "Armor" And "Craft" Buttons To Touch Inventory
    }

    // Display Slot Count
    if (feature_has("Display Slot Count In Creative Mode", server_enabled)) {
        patch((void *) 0x1e3f4, nop_patch);
        unsigned char slot_count_patch[4] = {0x18, 0x00, 0x00, 0xea}; // "b 0x27110"
        patch((void *) 0x270a8, slot_count_patch);
        patch((void *) 0x33954, nop_patch);
    }

    // Maximize Creative Inventory Stack Size
    if (feature_has("Maximize Creative Mode Inventory Stack Size", server_enabled)) {
        overwrite_calls(Inventory_setupDefault, Inventory_setupDefault_injection);
    }
}
