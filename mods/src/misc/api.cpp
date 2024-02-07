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
static void Minecraft_update_injection(Minecraft *minecraft) {
    // Call Original Method
    Minecraft_update_non_virtual(minecraft);

    // Run Functions
    handle_misc_update(minecraft);
}

// Run Functions On Tick
SETUP_CALLBACK(tick, Minecraft);
// Handle Custom Tick Behavior
static void Minecraft_tick_injection(Minecraft *minecraft, int32_t param_1, int32_t param_2) {
    // Call Original Method
    Minecraft_tick(minecraft, param_1, param_2);

    // Run Functions
    handle_misc_tick(minecraft);
}

// Run Functions On Recipes Setup
SETUP_CALLBACK(recipes_setup, Recipes);
// Handle Custom Recipes Setup Behavior
static Recipes *Recipes_injection(Recipes *recipes) {
    // Call Original Method
    Recipes_constructor(recipes);

    // Run Functions
    handle_misc_recipes_setup(recipes);

    // Return
    return recipes;
}

// Run Functions On Furnace Recipes Setup
SETUP_CALLBACK(furnace_recipes_setup, FurnaceRecipes);
// Handle Custom Furnace Recipes Setup Behavior
static FurnaceRecipes *FurnaceRecipes_injection(FurnaceRecipes *recipes) {
    // Call Original Method
    FurnaceRecipes_constructor(recipes);

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
static void Tile_initTiles_injection() {
    // Run Functions
    handle_misc_tiles_setup(NULL);

    // Call Original Method
    Tile_initTiles();
}

// Run Functions On Items Setup
SETUP_CALLBACK(items_setup, void);
// Handle Custom Items Setup Behavior
static void Item_initItems_injection() {
    // Call Original Method
    Item_initItems();

    // Run Functions
    handle_misc_items_setup(NULL);
}

// Run Functions On Language Setup
SETUP_CALLBACK(language_setup, void);
// Handle Custom Items Setup Behavior
static void I18n_loadLanguage_injection(AppPlatform *app, std::string language_name) {
    // Call Original Method
    I18n_loadLanguage(app, language_name);

    // Run Functions
    handle_misc_language_setup(NULL);
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
static void Gui_handleKeyPressed_injection(Gui *self, int key) {
    // Run Functions
    if (handle_misc_game_key_press(self->minecraft, key)) {
        return;
    }

    // Call Original Method
    Gui_handleKeyPressed(self, key);
}

// Init
void _init_misc_api() {
    // Handle Custom Update Behavior
    overwrite_calls((void *) Minecraft_update_non_virtual, (void *) Minecraft_update_injection);
    // Handle Custom Tick Behavior
    overwrite_calls((void *) Minecraft_tick, (void *) Minecraft_tick_injection);
    // Handle Custom Recipe Setup Behavior
    overwrite_calls((void *) Recipes_constructor, (void *) Recipes_injection);
    overwrite_calls((void *) FurnaceRecipes_constructor, (void *) FurnaceRecipes_injection);
    // Handle Custom Creative Inventory Setup Behavior
    overwrite_call((void *) 0x8e0fc, (void *) Inventory_setupDefault_FillingContainer_addItem_call_injection);
    // Handle Custom Item/Tile Init Behavior
    overwrite_calls((void *) Tile_initTiles, (void *) Tile_initTiles_injection);
    overwrite_calls((void *) Item_initItems, (void *) Item_initItems_injection);
    // Handle Custom Language Entries
    overwrite_calls((void *) I18n_loadLanguage, (void *) I18n_loadLanguage_injection);
    // Handle Key Presses
    overwrite_calls((void *) Gui_handleKeyPressed, (void *) Gui_handleKeyPressed_injection);
}
