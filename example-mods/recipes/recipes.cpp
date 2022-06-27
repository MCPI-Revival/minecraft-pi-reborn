// Headers

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <mods/misc/misc.h>

// Custom Crafting Recipes
static void Recipes_injection(unsigned char *recipes) {
    // Add
    Recipes_Type type1 = {
        .item = 0,
        .tile = 0,
        .instance = {
            .count = 1,
            .id = 12,
            .auxiliary = 0
        },
        .letter = 'a'
    };
    Recipes_Type type2 = {
        .item = 0,
        .tile = 0,
        .instance = {
            .count = 1,
            .id = 13,
            .auxiliary = 0
        },
        .letter = 'b'
    };
    ItemInstance result = {
        .count = 1,
        .id = 344,
        .auxiliary = 0
    };
    (*Recipes_addShapelessRecipe)(recipes, result, {type1, type2});
}

// Custom Furnace Recipes
static void FurnaceRecipes_injection(unsigned char *recipes) {
    // Add
    (*FurnaceRecipes_addFurnaceRecipe)(recipes, 49, {.count = 1, .id = 246, .auxiliary = 0});
}

// Init
__attribute__((constructor)) static void init_recipes() {
    // Log
    INFO("Loading Custom Recipes");

    // Setup
    misc_run_on_recipes_setup(Recipes_injection);
    misc_run_on_furnace_recipes_setup(FurnaceRecipes_injection);
}
