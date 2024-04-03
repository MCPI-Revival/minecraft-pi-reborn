#include <utility>
#include <vector>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/misc/misc.h>
#include "misc-internal.h"

// Callbacks
#define STORE_CALLBACK(name, type) \
    static std::vector<misc_update_function_##type##_t> &get_misc_##name##_functions() { \
        static std::vector<misc_update_function_##type##_t> functions; \
        return functions; \
    } \
    void misc_run_on_##name(misc_update_function_##type##_t function) { \
        get_misc_##name##_functions().push_back(function); \
    }
#define SETUP_CALLBACK(name, type) \
    STORE_CALLBACK(name, type) \
    static void handle_misc_##name(type *obj) { \
        for (misc_update_function_##type##_t function : get_misc_##name##_functions()) { \
            function(obj); \
        } \
    }

// Run Functions On Update
SETUP_CALLBACK(update, Minecraft);
// Handle Custom Update Behavior
static void Minecraft_update_injection(Minecraft_update_t original, Minecraft *minecraft) {
    // Call Original Method
    original(minecraft);

    // Run Functions
    handle_misc_update(minecraft);
}

// Run Functions On Tick
SETUP_CALLBACK(tick, Minecraft);
// Handle Custom Tick Behavior
static void Minecraft_tick_injection(Minecraft_tick_t original, Minecraft *minecraft, int32_t param_1, int32_t param_2) {
    // Call Original Method
    original(minecraft, param_1, param_2);

    // Run Functions
    handle_misc_tick(minecraft);
}

// Run Functions On Recipes Setup
SETUP_CALLBACK(recipes_setup, Recipes);
// Handle Custom Recipes Setup Behavior
static Recipes *Recipes_injection(Recipes_constructor_t original, Recipes *recipes) {
    // Call Original Method
    original(recipes);

    // Run Functions
    handle_misc_recipes_setup(recipes);

    // Return
    return recipes;
}

// Run Functions On Furnace Recipes Setup
SETUP_CALLBACK(furnace_recipes_setup, FurnaceRecipes);
// Handle Custom Furnace Recipes Setup Behavior
static FurnaceRecipes *FurnaceRecipes_injection(FurnaceRecipes_constructor_t original, FurnaceRecipes *recipes) {
    // Call Original Method
    original(recipes);

    // Run Functions
    handle_misc_furnace_recipes_setup(recipes);

    // Return
    return recipes;
}

// Run Functions On Creative Inventory Setup
SETUP_CALLBACK(creative_inventory_setup, FillingContainer);
// Handle Custom Creative Inventory Setup Behavior
static void Inventory_setupDefault_FillingContainer_addItem_call_injection(FillingContainer *filling_container, ItemInstance *item_instance) {
    // Call Original Method
    FillingContainer_addItem(filling_container, item_instance);

    // Run Functions
    handle_misc_creative_inventory_setup(filling_container);
}

// Run Functions On Tiles Setup
SETUP_CALLBACK(tiles_setup, void);
// Handle Custom Tiles Setup Behavior
static void Tile_initTiles_injection(Tile_initTiles_t original) {
    // Run Functions
    handle_misc_tiles_setup(nullptr);

    // Call Original Method
    original();
}

// Run Functions On Items Setup
SETUP_CALLBACK(items_setup, void);
// Handle Custom Items Setup Behavior
static void Item_initItems_injection(Item_initItems_t original) {
    // Call Original Method
    original();

    // Run Functions
    handle_misc_items_setup(nullptr);
}

// Run Functions On Language Setup
SETUP_CALLBACK(language_setup, void);
// Handle Custom Items Setup Behavior
static void I18n_loadLanguage_injection(I18n_loadLanguage_t original, AppPlatform *app, std::string language_name) {
    // Call Original Method
    original(app, std::move(language_name));

    // Run Functions
    handle_misc_language_setup(nullptr);
}

// Run Functions On GUI Key Press
STORE_CALLBACK(game_key_press, key_press)
static bool handle_misc_game_key_press(Minecraft *minecraft, int key) {
    for (misc_update_function_key_press_t function : get_misc_game_key_press_functions()) {
        if (function(minecraft, key)) {
            return true;
        }
    }
    return false;
}
// Handle Key Presses
static void Gui_handleKeyPressed_injection(Gui_handleKeyPressed_t original, Gui *self, int key) {
    // Run Functions
    if (handle_misc_game_key_press(self->minecraft, key)) {
        return;
    }

    // Call Original Method
    original(self, key);
}

// Init
void _init_misc_api() {
    // Handle Custom Update Behavior
    overwrite_virtual_calls(Minecraft_update, Minecraft_update_injection);
    // Handle Custom Tick Behavior
    overwrite_calls(Minecraft_tick, Minecraft_tick_injection);
    // Handle Custom Recipe Setup Behavior
    overwrite_calls(Recipes_constructor, Recipes_injection);
    overwrite_calls(FurnaceRecipes_constructor, FurnaceRecipes_injection);
    // Handle Custom Creative Inventory Setup Behavior
    overwrite_call((void *) 0x8e0fc, (void *) Inventory_setupDefault_FillingContainer_addItem_call_injection);
    // Handle Custom Item/Tile Init Behavior
    overwrite_calls(Tile_initTiles, Tile_initTiles_injection);
    overwrite_calls(Item_initItems, Item_initItems_injection);
    // Handle Custom Language Entries
    overwrite_calls(I18n_loadLanguage, I18n_loadLanguage_injection);
    // Handle Key Presses
    overwrite_calls(Gui_handleKeyPressed, Gui_handleKeyPressed_injection);
}
