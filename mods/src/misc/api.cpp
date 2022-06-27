#include <vector>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/misc/misc.h>
#include "misc-internal.h"

// Run Functions On Update
static std::vector<misc_update_function_t> &get_misc_update_functions() {
    static std::vector<misc_update_function_t> functions;
    return functions;
}
void misc_run_on_update(misc_update_function_t function) {
    get_misc_update_functions().push_back(function);
}
// Handle Custom Update Behavior
static void Minecraft_update_injection(unsigned char *minecraft) {
    // Call Original Method
    (*Minecraft_update)(minecraft);

    // Run Functions
    for (misc_update_function_t function : get_misc_update_functions()) {
        (*function)(minecraft);
    }
}

// Run Functions On Tick
static std::vector<misc_update_function_t> &get_misc_tick_functions() {
    static std::vector<misc_update_function_t> functions;
    return functions;
}
void misc_run_on_tick(misc_update_function_t function) {
    get_misc_tick_functions().push_back(function);
}
// Handle Custom Tick Behavior
static void Minecraft_tick_injection(unsigned char *minecraft, int32_t param_1, int32_t param_2) {
    // Call Original Method
    (*Minecraft_tick)(minecraft, param_1, param_2);

    // Run Functions
    for (misc_update_function_t function : get_misc_tick_functions()) {
        (*function)(minecraft);
    }
}

// Run Functions On Recipes Setup
static std::vector<misc_update_function_t> &get_misc_recipes_setup_functions() {
    static std::vector<misc_update_function_t> functions;
    return functions;
}
void misc_run_on_recipes_setup(misc_update_function_t function) {
    get_misc_recipes_setup_functions().push_back(function);
}
// Handle Custom Recipes Setup Behavior
static unsigned char *Recipes_injection(unsigned char *recipes) {
    // Call Original Method
    (*Recipes)(recipes);

    // Run Functions
    for (misc_update_function_t function : get_misc_recipes_setup_functions()) {
        (*function)(recipes);
    }

    // Return
    return recipes;
}

// Run Functions On Furnace Recipes Setup
static std::vector<misc_update_function_t> &get_misc_furnace_recipes_setup_functions() {
    static std::vector<misc_update_function_t> functions;
    return functions;
}
void misc_run_on_furnace_recipes_setup(misc_update_function_t function) {
    get_misc_furnace_recipes_setup_functions().push_back(function);
}
// Handle Custom Furnace Recipes Setup Behavior
static unsigned char *FurnaceRecipes_injection(unsigned char *recipes) {
    // Call Original Method
    (*FurnaceRecipes)(recipes);

    // Run Functions
    for (misc_update_function_t function : get_misc_furnace_recipes_setup_functions()) {
        (*function)(recipes);
    }

    // Return
    return recipes;
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
}
