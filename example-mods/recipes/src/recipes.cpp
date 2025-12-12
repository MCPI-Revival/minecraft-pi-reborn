#include <libreborn/log.h>

#include <symbols/Recipes.h>
#include <symbols/FurnaceRecipes.h>
#include <symbols/Item.h>

#include <mods/misc/misc.h>

// Custom Crafting Recipes
#define SAND 12
static void Recipes_injection(Recipes *recipes) {
    // Add
    constexpr Recipes_Type type1 = {
        .item = nullptr,
        .tile = nullptr,
        .instance = {
            .count = 1,
            .id = SAND,
            .auxiliary = 0
        },
        .letter = 'a'
    };
    constexpr Recipes_Type type2 = {
        .item = nullptr,
        .tile = nullptr,
        .instance = {
            .count = 1,
            .id = 13,
            .auxiliary = 0
        },
        .letter = 'b'
    };
    constexpr ItemInstance result = {
        .count = 1,
        .id = 344,
        .auxiliary = 0
    };
    const std::vector types = {type1, type2};
    recipes->addShapelessRecipe(result, types);
}

// Custom Furnace Recipes
static void FurnaceRecipes_injection(FurnaceRecipes *recipes) {
    // Add
    constexpr ItemInstance result = {
        .count = 1,
        .id = 246,
        .auxiliary = 0
    };
    recipes->addFurnaceRecipe(49, result);
}

// Init
__attribute__((constructor)) static void init_recipes() {
    // Log
    INFO("Loading Custom Recipes");

    // Setup
    misc_run_on_recipes_setup(Recipes_injection);
    misc_run_on_furnace_recipes_setup(FurnaceRecipes_injection);

    // Recipe Remainder
    misc_run_on_init([](Minecraft *) {
        Item::items[SAND]->craftingRemainingItem = Item::snowball;
    });
}
