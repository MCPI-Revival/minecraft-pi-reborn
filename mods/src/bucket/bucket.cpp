#include <libreborn/patch.h>

#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include <mods/misc/misc.h>
#include <mods/bucket/bucket.h>

// Custom Item
#define MILK_AUX 1
struct Bucket final : CustomItem {
    // Constructor
    explicit Bucket(const int id):
        CustomItem(id) {}

    // Description And Texture
    std::string getDescriptionId(const ItemInstance *item_instance) const override {
        if (item_instance->auxiliary == Tile::water->id) {
            return "item.bucketWater";
        } else if (item_instance->auxiliary == Tile::lava->id) {
            return "item.bucketLava";
        } else if (item_instance->auxiliary == MILK_AUX) {
            return "item.bucketMilk";
        } else {
            return "item.bucket";
        }
    }
    int getIcon(const int auxiliary) override {
        if (auxiliary == Tile::water->id) {
            return 75;
        } else if (auxiliary == Tile::lava->id) {
            return 76;
        } else if (auxiliary == MILK_AUX) {
            return 77;
        } else {
            return 74;
        }
    }

    // Filling
    bool _fill_bucket(ItemInstance *item_instance, const Player *player, const int new_auxiliary) const {
        Inventory *inventory = player->inventory;
        if (inventory->is_creative) {
            return true;
        }
        bool success = false;
        if (item_instance->count == 1) {
            item_instance->auxiliary = new_auxiliary;
            success = true;
        } else {
            ItemInstance new_item;
            new_item.id = self->id;
            new_item.count = 1;
            new_item.auxiliary = new_auxiliary;
            if (inventory->add(&new_item)) {
                // Added To Inventory
                success = true;
                item_instance->count -= 1;
            }
        }
        return success;
    }

    // Use Bucket
    bool useOn(ItemInstance *item_instance, Player *player, Level *level, int x, int y, int z, const int hit_side, MCPI_UNUSED float hit_x, MCPI_UNUSED float hit_y, MCPI_UNUSED float hit_z) override {
        if (item_instance->count < 1 || item_instance->auxiliary == MILK_AUX) {
            // Do Nothing
            return false;
        } else if (item_instance->auxiliary == 0) {
            // Use Empty Bucket
            int32_t new_auxiliary = 0;
            const int32_t tile = level->getTile(x, y, z);
            if (tile == Tile::calmWater->id) {
                new_auxiliary = Tile::water->id;
            } else if (tile == Tile::calmLava->id) {
                new_auxiliary = Tile::lava->id;
            }
            if (new_auxiliary != 0) {
                // Valid
                if (_fill_bucket(item_instance, player, new_auxiliary)) {
                    level->setTileAndData(x, y, z, 0, 0);
                    return true;
                } else {
                    return false;
                }
            } else {
                // Invalid
                return false;
            }
        } else {
            // Place
            switch (hit_side) {
                case 0: y -= 1; break;
                case 1: y += 1; break;
                case 2: z -= 1; break;
                case 3: z += 1; break;
                case 4: x -= 1; break;
                case 5: x += 1; break;
                default: {}
            }
            // Get Current Tile
            bool valid = false;
            const Material *material = level->getMaterial(x, y, z);
            if (material != nullptr) {
                valid = !material->isSolid();
            }
            if (item_instance->auxiliary != Tile::water->id && item_instance->auxiliary != Tile::lava->id) {
                valid = false;
            }
            if (valid) {
                level->setTileAndData(x, y, z, item_instance->auxiliary, 0);
                if (!player->inventory->is_creative) {
                    item_instance->auxiliary = 0;
                }
                return true;
            } else {
                return false;
            }
        }
    }

    // Milk
    int getUseDuration(ItemInstance *item_instance) override {
        if (item_instance->auxiliary == MILK_AUX) {
            return FoodItem_getUseDuration->get(false)(nullptr, item_instance);
        } else {
            return 0;
        }
    }
    ItemInstance useTimeDepleted(ItemInstance *item_instance, MCPI_UNUSED Level *level, Player *player) override {
        if (item_instance->auxiliary == MILK_AUX) {
            // Finish Drinking
            player->foodData.eat(0);
            // Set To An Empty Bucket
            if (!player->inventory->is_creative) {
                item_instance->auxiliary = 0;
                item_instance->count = 1;
            }
        }
        // Return
        return *item_instance;
    }
    int getUseAnimation() override {
        return 2;
    }
    [[nodiscard]] bool isFood() const override {
        return true;
    }
    ItemInstance *use(ItemInstance *item_instance, MCPI_UNUSED Level *level, Player *player) override {
        if (item_instance->auxiliary == MILK_AUX) {
            // Start Drinking
            player->startUsingItem(item_instance, getUseDuration(item_instance));
        }
        return item_instance;
    }

    // Crafting
    ItemInstance *getCraftingRemainingItem(ItemInstance *item_instance) override {
        if (item_instance->auxiliary == 0) {
            return nullptr;
        }
        ItemInstance *ret = new ItemInstance;
        ret->id = self->id;
        ret->count = item_instance->count;
        ret->auxiliary = 0;
        return ret;
    }
};
static Item *bucket = nullptr;

// Create Items
static Item *create_bucket(const int32_t id, const int32_t texture_x, const int32_t texture_y, const std::string &name) {
    // Construct
    Item *item = (new Bucket(id))->self;

    // Setup
    item->setIcon(texture_x, texture_y);
    item->setDescriptionId(name);
    item->is_stacked_by_data = true;
    item->category = 2;
    item->max_damage = 0;
    item->max_stack_size = 1;

    // Return
    return item;
}
static void Item_initItems_injection() {
    bucket = create_bucket(69, 10, 4, "bucket");
}

// Change Max Stack Size Based On Auxiliary
static int32_t ItemInstance_getMaxStackSize_injection(ItemInstance_getMaxStackSize_t original, const ItemInstance *item_instance) {
    if (item_instance->id == bucket->id && item_instance->auxiliary == 0) {
        // Custom Value
        return 16;
    } else {
        // Call Original Method
        return original(item_instance);
    }
}

// Milking
static bool Cow_interact_injection(Cow_interact_t original, Cow *self, Player *player) {
    ItemInstance *item = player->inventory->getSelected();
    if (item && item->id == bucket->id && item->auxiliary == 0) {
        // Fill With Milk
        custom_get<Bucket>(bucket)->_fill_bucket(item, player, 1);
        return true;
    }
    return original(self, player);
}
static void CreatorMode_releaseUsingItem_ItemInstance_setAuxValue_injection(ItemInstance *self, const int aux) {
    if (self->id != bucket->id && self->auxiliary != MILK_AUX) {
        self->auxiliary = aux;
    }
}

// Creative Inventory
static void inventory_add_item(FillingContainer *inventory, Item *item, int32_t auxiliary) {
    ItemInstance *item_instance = new ItemInstance;
    item_instance = item_instance->constructor_item_extra(item, 1, auxiliary);
    inventory->addItem(item_instance);
}
static void Inventory_setupDefault_FillingContainer_addItem_injection(FillingContainer *filling_container) {
    inventory_add_item(filling_container, bucket, 0);
    inventory_add_item(filling_container, bucket, Tile::water->id);
    inventory_add_item(filling_container, bucket, Tile::lava->id);
    inventory_add_item(filling_container, bucket, 1);
}

// Make Liquids Selectable
static bool is_holding_bucket = false;
static HitResult Mob_pick_Level_clip_injection(Level *level, const Vec3 &param_1, const Vec3 &param_2, MCPI_UNUSED bool clip_liquids, bool clip_hitboxes) {
    // Call Original Method
    return level->clip(param_1, param_2, is_holding_bucket, clip_hitboxes);
}
static void handle_tick(const Minecraft *minecraft) {
    LocalPlayer *player = minecraft->player;
    if (player != nullptr) {
        // Get Selected Slot
        const int32_t selected_slot = misc_get_real_selected_slot((Player *) player);
        Inventory *inventory = player->inventory;

        // Get Item
        const ItemInstance *inventory_item = inventory->getItem(selected_slot);
        // Check
        is_holding_bucket = inventory_item != nullptr && inventory_item->id == bucket->id && inventory_item->auxiliary == 0;
    }
}

// Prevent Breaking Liquid
static bool is_calm_liquid(const int32_t id) {
    if (id == Tile::calmWater->id) {
        return true;
    } else if (id == Tile::calmLava->id) {
        return true;
    } else {
        return false;
    }
}
static void Minecraft_handleMouseDown_injection(Minecraft_handleMouseDown_t original, Minecraft *minecraft, const int param_1, bool can_destroy) {
    // Check
    Level *level = minecraft->level;
    if (level != nullptr) {
        const int32_t x = minecraft->hit_result.x;
        const int32_t y = minecraft->hit_result.y;
        const int32_t z = minecraft->hit_result.z;
        const int32_t tile = level->getTile(x, y, z);
        if (is_calm_liquid(tile)) {
            can_destroy = false;
        }
    }

    // Call Original Method
    original(minecraft, param_1, can_destroy);
}

// Custom Crafting Recipes
static void Recipes_injection(Recipes *recipes) {
    // Add
    constexpr Recipes_Type type1 = {
        .item = nullptr,
        .tile = nullptr,
        .instance = {
            .count = 3,
            .id = 265,
            .auxiliary = 0
        },
        .letter = '#'
    };
    const ItemInstance result = {
        .count = 1,
        .id = bucket->id,
        .auxiliary = 0
    };
    const std::string line1 = "# #";
    const std::string line2 = " # ";
    const std::vector types = {type1};
    recipes->addShapedRecipe_2(result, line1, line2, types);
}

// Custom Furnace Fuel
static int32_t FurnaceTileEntity_getBurnDuration_injection(FurnaceTileEntity_getBurnDuration_t original, const ItemInstance &item_instance) {
    if (item_instance.count > 0 && item_instance.id == bucket->id && item_instance.auxiliary == Tile::lava->id) {
        return 20000;
    } else {
        // Call Original Method
        return original(item_instance);
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
static void Language_injection() {
    I18n::_strings["item.bucketMilk.name"] = "Milk Bucket";
}

// Init
bool buckets_enabled = false;
void init_bucket() {
    // Add Buckets
    buckets_enabled = feature_has("Add Buckets", server_is_not_vanilla_compatible);
    if (buckets_enabled) {
        // Add Items
        misc_run_on_items_setup(Item_initItems_injection);
        // Change Max Stack Size Based On Auxiliary
        overwrite_calls(ItemInstance_getMaxStackSize, ItemInstance_getMaxStackSize_injection);
        // Enable Milking
        overwrite_calls(Cow_interact, Cow_interact_injection);
        overwrite_call((void *) 0x19f1c, ItemInstance_setAuxValue, CreatorMode_releaseUsingItem_ItemInstance_setAuxValue_injection);
        // Creative Inventory
        misc_run_on_creative_inventory_setup(Inventory_setupDefault_FillingContainer_addItem_injection);
        // Make Liquids Selectable
        overwrite_call((void *) 0x7f5b0, Level_clip, Mob_pick_Level_clip_injection);
        misc_run_on_tick(handle_tick);
        // Prevent Breaking Liquid
        overwrite_calls(Minecraft_handleMouseDown, Minecraft_handleMouseDown_injection);
        // Custom Crafting Recipes
        misc_run_on_recipes_setup(Recipes_injection);
        // Custom Furnace Fuel
        overwrite_calls(FurnaceTileEntity_getBurnDuration, FurnaceTileEntity_getBurnDuration_injection);
        overwrite_call((void *) 0xd351c, ItemInstance_setNull, FurnaceTileEntity_tick_ItemInstance_setNull_injection);
        // Language For Milk
        misc_run_on_language_setup(Language_injection);
    }
}
