#include <vector>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/misc/misc.h>
#include "misc-internal.h"

// Callbacks
#define SETUP_CALLBACK(name) \
    static std::vector<misc_update_function_t> &get_misc_##name##_functions() { \
        static std::vector<misc_update_function_t> functions; \
        return functions; \
    } \
    static void handle_misc_##name(unsigned char *obj) { \
        for (misc_update_function_t function : get_misc_##name##_functions()) { \
            (*function)(obj); \
        } \
    } \
    void misc_run_on_##name(misc_update_function_t function) { \
        get_misc_##name##_functions().push_back(function); \
    }

// Run Functions On Update
SETUP_CALLBACK(update);
// Handle Custom Update Behavior
static void Minecraft_update_injection(unsigned char *minecraft) {
    // Call Original Method
    (*Minecraft_update)(minecraft);

    // Run Functions
    handle_misc_update(minecraft);
}

// Run Functions On Tick
SETUP_CALLBACK(tick);
// Handle Custom Tick Behavior
static void Minecraft_tick_injection(unsigned char *minecraft, int32_t param_1, int32_t param_2) {
    // Call Original Method
    (*Minecraft_tick)(minecraft, param_1, param_2);

    // Run Functions
    handle_misc_tick(minecraft);
}

// Run Functions On Recipes Setup
SETUP_CALLBACK(recipes_setup);
// Handle Custom Recipes Setup Behavior
static unsigned char *Recipes_injection(unsigned char *recipes) {
    // Call Original Method
    (*Recipes)(recipes);

    // Run Functions
    handle_misc_recipes_setup(recipes);

    // Return
    return recipes;
}

// Run Functions On Furnace Recipes Setup
SETUP_CALLBACK(furnace_recipes_setup);
// Handle Custom Furnace Recipes Setup Behavior
static unsigned char *FurnaceRecipes_injection(unsigned char *recipes) {
    // Call Original Method
    (*FurnaceRecipes)(recipes);

    // Run Functions
    handle_misc_furnace_recipes_setup(recipes);

    // Return
    return recipes;
}

// Run Functions On Creative Inventory Setup
SETUP_CALLBACK(creative_inventory_setup);
// Handle Custom Creative Inventory Setup Behavior
static void Inventory_setupDefault_FillingContainer_addItem_call_injection(unsigned char *filling_container, ItemInstance *item_instance) {
    // Call Original Method
    (*FillingContainer_addItem)(filling_container, item_instance);
    // Run Functions
    handle_misc_creative_inventory_setup(filling_container);
}

// Run Functions On Tiles Setup
SETUP_CALLBACK(tiles_setup);
SETUP_CALLBACK(internal_after_tiles_setup);
// Handle Custom Tiles Setup Behavior
static void Tile_initTiles_injection() {
    // Run Functions
    handle_misc_tiles_setup(NULL);

    // Call Original Method
    (*Tile_initTiles)();

    // Run Functions
    handle_misc_internal_after_tiles_setup(NULL);
}

// Run Functions On Items Setup
SETUP_CALLBACK(items_setup);
// Handle Custom Items Setup Behavior
static void Item_initItems_injection() {
    // Run Functions
    handle_misc_items_setup(NULL);

    // Call Original Method
    (*Item_initItems)();
}

// Init
void _init_misc_api() {
    // Handle Custom Update Behavior
    overwrite_calls((void *) Minecraft_update, (void *) Minecraft_update_injection);
    // Handle Custom Tick Behavior
    overwrite_calls((void *) Minecraft_tick, (void *) Minecraft_tick_injection);
    // Handle Custom Recipe Setup Behavior
    overwrite_calls((void *) Recipes, (void *) Recipes_injection);
    overwrite_calls((void *) FurnaceRecipes, (void *) FurnaceRecipes_injection);
    // Handle Custom Creative Inventory Setup Behavior
    overwrite_call((void *) 0xcdcf4, (void *) Inventory_setupDefault_FillingContainer_addItem_call_injection);
    // Handle Custom Item/Tile Init Behavior
    overwrite_calls((void *) Tile_initTiles, (void *) Tile_initTiles_injection);
    overwrite_calls((void *) Item_initItems, (void *) Item_initItems_injection);
}
