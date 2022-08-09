#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include <mods/misc/misc.h>

// Items
unsigned char *bucket = NULL;

// Description And Texture
static std::string BucketItem_getDescriptionId(__attribute__((unused)) unsigned char *item, const ItemInstance *item_instance) {
    if (item_instance->auxiliary == *(int32_t *) (*Tile_water + Tile_id_property_offset)) {
        return "item.bucketWater";
    } else if (item_instance->auxiliary == *(int32_t *) (*Tile_lava + Tile_id_property_offset)) {
        return "item.bucketLava";
    } else {
        return "item.bucket";
    }
}
static int32_t BucketItem_getIcon(__attribute__((unused)) unsigned char *item, int32_t auxiliary) {
    if (auxiliary == *(int32_t *) (*Tile_water + Tile_id_property_offset)) {
        return 75;
    } else if (auxiliary == *(int32_t *) (*Tile_lava + Tile_id_property_offset)) {
        return 76;
    } else {
        return 74;
    }
}

// Use Bucket
static int32_t BucketItem_useOn(__attribute__((unused)) unsigned char *item, ItemInstance *item_instance, unsigned char *player, unsigned char *level, int32_t x, int32_t y, int32_t z, int32_t hit_side, __attribute__((unused)) float hit_x, __attribute__((unused)) float hit_y, __attribute__((unused)) float hit_z) {
    if (item_instance->count < 1) {
        return 0;
    } else if (item_instance->auxiliary == 0) {
        // Empty Bucket
        int32_t new_auxiliary = 0;
        int32_t tile = (*Level_getTile)(level, x, y, z);
        if (tile == *(int32_t *) (*Tile_calmWater + Tile_id_property_offset)) {
            new_auxiliary = *(int32_t *) (*Tile_water + Tile_id_property_offset);
        } else if (tile == *(int32_t *) (*Tile_calmLava + Tile_id_property_offset)) {
            new_auxiliary = *(int32_t *) (*Tile_lava + Tile_id_property_offset);
        }
        if (new_auxiliary != 0) {
            // Valid
            bool success = false;
            if (item_instance->count == 1) {
                item_instance->auxiliary = new_auxiliary;
                success = true;
            } else {
                ItemInstance new_item;
                new_item.id = *(int32_t *) (bucket + Item_id_property_offset);
                new_item.count = 1;
                new_item.auxiliary = new_auxiliary;
                unsigned char *inventory = *(unsigned char **) (player + Player_inventory_property_offset);
                unsigned char *inventory_vtable = *(unsigned char **) inventory;
                FillingContainer_add_t FillingContainer_add = *(FillingContainer_add_t *) (inventory_vtable + FillingContainer_add_vtable_offset);
                if ((*FillingContainer_add)(inventory, &new_item)) {
                    // Added To Inventory
                    success = true;
                    item_instance->count -= 1;
                }
            }
            if (success) {
                (*Level_setTileAndData)(level, x, y, z, 0, 0);
                return 1;
            } else {
                return 0;
            }
        } else {
            // Invalid
            return 0;
        }
    } else {
        // Place
        switch (hit_side) {
            case 0: {
                y -= 1;
                break;
            }
            case 1: {
                y += 1;
                break;
            }
            case 2: {
                z -= 1;
                break;
            }
            case 3: {
                z += 1;
                break;
            }
            case 4: {
                x -= 1;
                break;
            }
            case 5: {
                x += 1;
                break;
            }
        }
        // Get Current Tile
        bool valid = false;
        unsigned char *material = (*Level_getMaterial)(level, x, y, z);
        if (material != NULL) {
            unsigned char *material_vtable = *(unsigned char **) material;
            Material_isSolid_t Material_isSolid = *(Material_isSolid_t *) (material_vtable + Material_isSolid_vtable_offset);
            valid = !(*Material_isSolid)(material);
        }
        if (item_instance->auxiliary != *(int32_t *) (*Tile_water + Tile_id_property_offset) && item_instance->auxiliary != *(int32_t *) (*Tile_lava + Tile_id_property_offset)) {
            valid = false;
        }
        if (valid) {
            (*Level_setTileAndData)(level, x, y, z, item_instance->auxiliary, 0);
            item_instance->auxiliary = 0;
            return 1;
        } else {
            return 0;
        }
    }
}

// Bucket VTable
static unsigned char *get_bucket_vtable() {
    static unsigned char *vtable = NULL;
    if (vtable == NULL) {
        // Init
        vtable = (unsigned char *) malloc(ITEM_VTABLE_SIZE);
        ALLOC_CHECK(vtable);
        // Copy Old VTable
        memcpy((void *) vtable, (void *) Item_vtable, ITEM_VTABLE_SIZE);

        // Modify
        *(Item_getDescriptionId_t *) (vtable + Item_getDescriptionId_vtable_offset) = BucketItem_getDescriptionId;
        *(Item_getIcon_t *) (vtable + Item_getIcon_vtable_offset) = BucketItem_getIcon;
        *(Item_useOn_t *) (vtable + Item_useOn_vtable_offset) = BucketItem_useOn;
    }
    return vtable;
}
__attribute__((destructor)) static void free_bucket_vtable() {
    free(get_bucket_vtable());
}

// Create Items
static unsigned char *create_bucket(int32_t id, int32_t texture_x, int32_t texture_y, const char *name) {
    // Construct
    unsigned char *item = (unsigned char *) ::operator new(ITEM_SIZE);
    ALLOC_CHECK(item);
    (*Item)(item, id);

    // Set VTable
    *(unsigned char **) item = get_bucket_vtable();

    // Get Functions
    unsigned char *vtable = *(unsigned char **) item;
    Item_setIcon_t Item_setIcon = *(Item_setIcon_t *) (vtable + Item_setIcon_vtable_offset);
    Item_setDescriptionId_t Item_setDescriptionId = *(Item_setDescriptionId_t *) (vtable + Item_setDescriptionId_vtable_offset);

    // Setup
    (*Item_setIcon)(item, texture_x, texture_y);
    (*Item_setDescriptionId)(item, name);
    *(int32_t *) (item + Item_is_stacked_by_data_property_offset) = 1;
    *(int32_t *) (item + Item_category_property_offset) = 2;
    *(int32_t *) (item + Item_max_damage_property_offset) = 0;
    *(int32_t *) (item + Item_max_stack_size_property_offset) = 1;

    // Return
    return item;
}
static void Item_initItems_injection(__attribute__((unused)) unsigned char *null) {
    bucket = create_bucket(69, 10, 4, "bucket");
}

// Change Max Stack Size Based On Auxiliary
static int32_t ItemInstance_getMaxStackSize_injection(ItemInstance *item_instance) {
    if (item_instance->id == *(int32_t *) (bucket + Item_id_property_offset) && item_instance->auxiliary == 0) {
        // Custom Value
        return 16;
    } else {
        // Call Original Method
        return (*ItemInstance_getMaxStackSize)(item_instance);
    }
}

// Creative Inventory
static void inventory_add_item(unsigned char *inventory, unsigned char *item, int32_t auxiliary) {
    ItemInstance *item_instance = new ItemInstance;
    ALLOC_CHECK(item_instance);
    item_instance = (*ItemInstance_constructor_item_extra)(item_instance, item, 1, auxiliary);
    (*FillingContainer_addItem)(inventory, item_instance);
}
static void Inventory_setupDefault_FillingContainer_addItem_call_injection(unsigned char *filling_container) {
    inventory_add_item(filling_container, bucket, 0);
    inventory_add_item(filling_container, bucket, *(int32_t *) (*Tile_water + Tile_id_property_offset));
    inventory_add_item(filling_container, bucket, *(int32_t *) (*Tile_lava + Tile_id_property_offset));
}

// Make Liquids Selectable
static bool is_holding_bucket = false;
static void Mob_pick_Level_clip_injection(unsigned char *level, unsigned char *param_1, unsigned char *param_2, bool param_3, __attribute__((unused)) bool clip_liquids) {
    // Call Original Method
    (*Level_clip)(level, param_1, param_2, param_3, is_holding_bucket);
}
static void handle_tick(unsigned char *minecraft) {
    unsigned char *player = *(unsigned char **) (minecraft + Minecraft_player_property_offset);
    if (player != NULL) {
        // Get Selected Slot
        int32_t selected_slot = misc_get_real_selected_slot(player);
        unsigned char *inventory = *(unsigned char **) (player + Player_inventory_property_offset);

        // Prepare
        unsigned char *inventory_vtable = *(unsigned char **) inventory;
        FillingContainer_getItem_t FillingContainer_getItem = *(FillingContainer_getItem_t *) (inventory_vtable + FillingContainer_getItem_vtable_offset);

        // Get Item
        ItemInstance *inventory_item = (*FillingContainer_getItem)(inventory, selected_slot);
        // Check
        is_holding_bucket = inventory_item != NULL && inventory_item->id == (*(int32_t *) (bucket + Item_id_property_offset)) && inventory_item->auxiliary == 0;
    }
}

// Prevent Breaking Liquid
static bool is_calm_liquid(int32_t id) {
    if (id == *(int32_t *) (*Tile_calmWater + Tile_id_property_offset)) {
        return true;
    } else if (id == *(int32_t *) (*Tile_calmLava + Tile_id_property_offset)) {
        return true;
    } else {
        return false;
    }
}
static void Minecraft_handleMouseDown_injection(unsigned char *minecraft, int param_1, bool can_destroy) {
    // Check
    unsigned char *level = *(unsigned char **) (minecraft + Minecraft_level_property_offset);
    if (level != NULL) {
        int32_t x = *(int32_t *) (minecraft + Minecraft_targeted_x_property_offset);
        int32_t y = *(int32_t *) (minecraft + Minecraft_targeted_y_property_offset);
        int32_t z = *(int32_t *) (minecraft + Minecraft_targeted_z_property_offset);
        int32_t tile = (*Level_getTile)(level, x, y, z);
        if (is_calm_liquid(tile)) {
            can_destroy = false;
        }
    }

    // Call Original Method
    (*Minecraft_handleMouseDown)(minecraft, param_1, can_destroy);
}

// Custom Crafting Recipes
static void Recipes_injection(unsigned char *recipes) {
    // Add
    Recipes_Type type1 = {
        .item = 0,
        .tile = 0,
        .instance = {
            .count = 3,
            .id = 265,
            .auxiliary = 0
        },
        .letter = '#'
    };
    ItemInstance result = {
        .count = 1,
        .id = (*(int32_t *) (bucket + Item_id_property_offset)),
        .auxiliary = 0
    };
    (*Recipes_addShapedRecipe_2)(recipes, result, "# #", " # ", {type1});
}

// Custom Furnace Fuel
static int32_t FurnaceTileEntity_getBurnDuration_injection(ItemInstance const& item_instance) {
    if (item_instance.count > 0 && item_instance.id == (*(int32_t *) (bucket + Item_id_property_offset)) && item_instance.auxiliary == (*(int32_t *) (*Tile_lava + Tile_id_property_offset))) {
        return 20000;
    } else {
        // Call Original Method
        return (*FurnaceTileEntity_getBurnDuration)(item_instance);
    }
}
static void FurnaceTileEntity_tick_ItemInstance_setNull_injection(ItemInstance *item_instance) {
    // Replace Lava Bucket With Empty Bucket When It Burns Out
    if (item_instance->id == (*(int32_t *) (bucket + Item_id_property_offset))) {
        item_instance->auxiliary = 0;
    } else {
        // Original Behavior
        item_instance->count = 0;
        item_instance->id = 0;
        item_instance->auxiliary = 0;
    }
}

// Init
void init_bucket() {
    // Add Buckets
    if (feature_has("Add Buckets", server_enabled)) {
        // Add Items
        misc_run_on_items_setup(Item_initItems_injection);
        // Change Max Stack Size Based On Auxiliary
        overwrite_calls((void *) ItemInstance_getMaxStackSize, (void *) ItemInstance_getMaxStackSize_injection);
        // Creative Inventory
        misc_run_on_creative_inventory_setup(Inventory_setupDefault_FillingContainer_addItem_call_injection);
        // Make Liquids Selectable
        overwrite_call((void *) 0x7f5b0, (void *) Mob_pick_Level_clip_injection);
        misc_run_on_tick(handle_tick);
        // Prevent Breaking Liquid
        overwrite_calls((void *) Minecraft_handleMouseDown, (void *) Minecraft_handleMouseDown_injection);
        // Custom Crafting Recipes
        misc_run_on_recipes_setup(Recipes_injection);
        // Custom Furnace Fuel
        overwrite_calls((void *) FurnaceTileEntity_getBurnDuration, (void *) FurnaceTileEntity_getBurnDuration_injection);
        overwrite_call((void *) 0xd351c, (void *) FurnaceTileEntity_tick_ItemInstance_setNull_injection);
    }
}
