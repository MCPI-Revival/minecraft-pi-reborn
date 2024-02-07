#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include <mods/misc/misc.h>

// Items
static FoodItem *bucket = NULL;

// Description And Texture
static std::string BucketItem_getDescriptionId(__attribute__((unused)) FoodItem *item, ItemInstance *item_instance) {
    if (item_instance->auxiliary == Tile_water->id) {
        return "item.bucketWater";
    } else if (item_instance->auxiliary == Tile_lava->id) {
        return "item.bucketLava";
    } else if (item_instance->auxiliary == 1) {
        return "item.bucketMilk";
    } else {
        return "item.bucket";
    }
}
static int32_t BucketItem_getIcon(__attribute__((unused)) FoodItem *item, int32_t auxiliary) {
    if (auxiliary == Tile_water->id) {
        return 75;
    } else if (auxiliary == Tile_lava->id) {
        return 76;
    } else if (auxiliary == 1) {
        return 77;
    } else {
        return 74;
    }
}

// Filling
static bool fill_bucket(ItemInstance *item_instance, Player *player, int new_auxiliary) {
    bool success = false;
    if (item_instance->count == 1) {
        item_instance->auxiliary = new_auxiliary;
        success = true;
    } else {
        ItemInstance new_item;
        new_item.id = bucket->id;
        new_item.count = 1;
        new_item.auxiliary = new_auxiliary;
        Inventory *inventory = player->inventory;
        if (inventory->vtable->add(inventory, &new_item)) {
            // Added To Inventory
            success = true;
            item_instance->count -= 1;
        }
    }
    return success;
}


// Use Bucket
static int32_t BucketItem_useOn(__attribute__((unused)) FoodItem *item, ItemInstance *item_instance, Player *player, Level *level, int32_t x, int32_t y, int32_t z, int32_t hit_side, __attribute__((unused)) float hit_x, __attribute__((unused)) float hit_y, __attribute__((unused)) float hit_z) {
    if (item_instance->count < 1 || item_instance->auxiliary == 1) {
        return 0;
    } else if (item_instance->auxiliary == 0) {
        // Empty Bucket
        int32_t new_auxiliary = 0;
        int32_t tile = level->vtable->getTile(level, x, y, z);
        if (tile == Tile_calmWater->id) {
            new_auxiliary = Tile_water->id;
        } else if (tile == Tile_calmLava->id) {
            new_auxiliary = Tile_lava->id;
        }
        if (new_auxiliary != 0) {
            // Valid
            if (fill_bucket(item_instance, player, new_auxiliary)) {
                Level_setTileAndData(level, x, y, z, 0, 0);
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
        Material *material = level->vtable->getMaterial(level, x, y, z);
        if (material != NULL) {
            valid = !material->vtable->isSolid(material);
        }
        if (item_instance->auxiliary != Tile_water->id && item_instance->auxiliary != Tile_lava->id) {
            valid = false;
        }
        if (valid) {
            Level_setTileAndData(level, x, y, z, item_instance->auxiliary, 0);
            item_instance->auxiliary = 0;
            return 1;
        } else {
            return 0;
        }
    }
}

static int BucketItem_getUseDuration(__attribute__((unused)) FoodItem *item, ItemInstance *item_instance) {
    if (item_instance->auxiliary == 1) {
        return 0x20;
    }
    return 0;
}

static ItemInstance BucketItem_useTimeDepleted(FoodItem *item, ItemInstance *item_instance, Level *level, Player *player) {
    if (item_instance->auxiliary == 1) {
        *item_instance = FoodItem_useTimeDepleted_non_virtual(item, item_instance, level, player);
        // Set it to a empty bucket
        item_instance->auxiliary = 0;
        item_instance->count = 1;
    }
    return *item_instance;
}

static int BucketItem_getUseAnimation(__attribute__((unused)) FoodItem *item) {
    return 2;
}

static bool BucketItem_isFood(__attribute__((unused)) FoodItem *item) {
    return true;
}

static ItemInstance *BucketItem_use(FoodItem *item, ItemInstance *item_instance, __attribute__((unused)) Level *level, Player *player) {
    if (item_instance->auxiliary == 1) {
        return (*FoodItem_use_vtable_addr)(item, item_instance, level, player);
    }
    return item_instance;
}

static ItemInstance *BucketItem_getCraftingRemainingItem(FoodItem *item, ItemInstance *item_instance) {
    if (item_instance->auxiliary == 0) {
        return NULL;
    }
    ItemInstance *ret = alloc_ItemInstance();
    ret->id = item->id;
    ret->count = item_instance->count;
    ret->auxiliary = 0;
    return ret;
}

// Bucket VTable
CUSTOM_VTABLE(bucket, FoodItem) {
    vtable->getDescriptionId = BucketItem_getDescriptionId;
    vtable->getIcon = BucketItem_getIcon;
    vtable->useOn = BucketItem_useOn;
    vtable->getUseDuration = BucketItem_getUseDuration;
    vtable->useTimeDepleted = BucketItem_useTimeDepleted;
    vtable->getUseAnimation = BucketItem_getUseAnimation;
    vtable->isFood = BucketItem_isFood;
    vtable->use = BucketItem_use;
    vtable->getCraftingRemainingItem = BucketItem_getCraftingRemainingItem;
}

// Create Items
static FoodItem *create_bucket(int32_t id, int32_t texture_x, int32_t texture_y, std::string name) {
    // Construct
    FoodItem *item = alloc_FoodItem();
    ALLOC_CHECK(item);
    Item_constructor((Item *) item, id);

    // Set VTable
    item->vtable = get_bucket_vtable();

    // Setup
    item->vtable->setIcon(item, texture_x, texture_y);
    item->vtable->setDescriptionId(item, &name);
    item->is_stacked_by_data = 1;
    item->category = 2;
    item->max_damage = 0;
    item->max_stack_size = 1;
    item->nutrition = 0;
    item->unknown_param_1 = 0.6;
    item->meat = false;

    // Return
    return item;
}
static void Item_initItems_injection(__attribute__((unused)) void *null) {
    bucket = create_bucket(69, 10, 4, "bucket");
}

// Change Max Stack Size Based On Auxiliary
static int32_t ItemInstance_getMaxStackSize_injection(ItemInstance *item_instance) {
    if (item_instance->id == bucket->id && item_instance->auxiliary == 0) {
        // Custom Value
        return 16;
    } else {
        // Call Original Method
        return ItemInstance_getMaxStackSize(item_instance);
    }
}

// Milking
bool Cow_interact_injection(Cow *self, Player *player) {
    ItemInstance *item = Inventory_getSelected(player->inventory);
    if (item && item->id == bucket->id && item->auxiliary == 0) {
        // Fill with milk
        fill_bucket(item, player, 1);
        return true;
    }
    return Cow_interact_non_virtual(self, player);
}

// Creative Inventory
static void inventory_add_item(FillingContainer *inventory, FoodItem *item, int32_t auxiliary) {
    ItemInstance *item_instance = new ItemInstance;
    ALLOC_CHECK(item_instance);
    item_instance = ItemInstance_constructor_item_extra(item_instance, (Item *) item, 1, auxiliary);
    FillingContainer_addItem(inventory, item_instance);
}
static void Inventory_setupDefault_FillingContainer_addItem_call_injection(FillingContainer *filling_container) {
    inventory_add_item(filling_container, bucket, 0);
    inventory_add_item(filling_container, bucket, Tile_water->id);
    inventory_add_item(filling_container, bucket, Tile_lava->id);
    inventory_add_item(filling_container, bucket, 1);
}

// Make Liquids Selectable
static bool is_holding_bucket = false;
static HitResult Mob_pick_Level_clip_injection(Level *level, unsigned char *param_1, unsigned char *param_2, __attribute__((unused)) bool clip_liquids, bool param_3) {
    // Call Original Method
    return Level_clip(level, param_1, param_2, is_holding_bucket, param_3);
}
static void handle_tick(Minecraft *minecraft) {
    LocalPlayer *player = minecraft->player;
    if (player != NULL) {
        // Get Selected Slot
        int32_t selected_slot = misc_get_real_selected_slot((Player *) player);
        Inventory *inventory = player->inventory;

        // Get Item
        ItemInstance *inventory_item = inventory->vtable->getItem(inventory, selected_slot);
        // Check
        is_holding_bucket = inventory_item != NULL && inventory_item->id == bucket->id && inventory_item->auxiliary == 0;
    }
}

// Prevent Breaking Liquid
static bool is_calm_liquid(int32_t id) {
    if (id == Tile_calmWater->id) {
        return true;
    } else if (id == Tile_calmLava->id) {
        return true;
    } else {
        return false;
    }
}
static void Minecraft_handleMouseDown_injection(Minecraft *minecraft, int param_1, bool can_destroy) {
    // Check
    Level *level = minecraft->level;
    if (level != NULL) {
        int32_t x = minecraft->hit_result.x;
        int32_t y = minecraft->hit_result.y;
        int32_t z = minecraft->hit_result.z;
        int32_t tile = level->vtable->getTile(level, x, y, z);
        if (is_calm_liquid(tile)) {
            can_destroy = false;
        }
    }

    // Call Original Method
    Minecraft_handleMouseDown(minecraft, param_1, can_destroy);
}

// Custom Crafting Recipes
static void Recipes_injection(Recipes *recipes) {
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
        .id = bucket->id,
        .auxiliary = 0
    };
    std::string line1 = "# #";
    std::string line2 = " # ";
    std::vector<Recipes_Type> types = {type1};
    Recipes_addShapedRecipe_2(recipes, &result, &line1, &line2, &types);
}

// Custom Furnace Fuel
static int32_t FurnaceTileEntity_getBurnDuration_injection(ItemInstance *item_instance) {
    if (item_instance->count > 0 && item_instance->id == bucket->id && item_instance->auxiliary == Tile_lava->id) {
        return 20000;
    } else {
        // Call Original Method
        return FurnaceTileEntity_getBurnDuration(item_instance);
    }
}
static void FurnaceTileEntity_tick_ItemInstance_setNull_injection(ItemInstance *item_instance) {
    // Replace Lava Bucket With Empty Bucket When It Burns Out
    if (item_instance->id == bucket->id) {
        item_instance->auxiliary = 0;
    } else {
        // Original Behavior
        item_instance->count = 0;
        item_instance->id = 0;
        item_instance->auxiliary = 0;
    }
}

// Add the bucket name to the language file
static void Language_injection(__attribute__((unused)) void *null) {
    I18n__strings.insert(std::make_pair("item.bucketMilk.name", "Milk Bucket"));
}

// Init
bool buckets_enabled = false;
void init_bucket() {
    // Add Buckets
    buckets_enabled = feature_has("Add Buckets", server_enabled);
    if (buckets_enabled) {
        // Add Items
        misc_run_on_items_setup(Item_initItems_injection);
        // Change Max Stack Size Based On Auxiliary
        overwrite_calls((void *) ItemInstance_getMaxStackSize, (void *) ItemInstance_getMaxStackSize_injection);
        // Enable milking
        patch_address((void *) Cow_interact_vtable_addr, (void *) Cow_interact_injection);
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
        // Language for milk
        misc_run_on_language_setup(Language_injection);
    }
}
