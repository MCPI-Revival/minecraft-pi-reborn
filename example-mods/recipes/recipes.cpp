#include <libreborn/patch.h>

#include <symbols/minecraft.h>

#include <mods/misc/misc.h>

// Custom Crafting Recipes
#define SAND 12
static void Recipes_injection(Recipes *recipes) {
    // Add
    Recipes_Type type1 = {
        .item = 0,
        .tile = 0,
        .instance = {
            .count = 1,
            .id = SAND,
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
    std::vector<Recipes_Type> types = {type1, type2};
    recipes->addShapelessRecipe(result, types);
}

// Custom Furnace Recipes
static void FurnaceRecipes_injection(FurnaceRecipes *recipes) {
    // Add
    ItemInstance result = {
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
    overwrite_calls(Minecraft_init, [](Minecraft_init_t original, Minecraft *self) {
        original(self);
        Item::items[SAND]->craftingRemainingItem = Item::snowball;
    });
}
