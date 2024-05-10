#pragma once

#include <cstdint>

#include <symbols/minecraft.h>

extern "C" {
int32_t misc_get_real_selected_slot(Player *player);
void misc_render_background(int color, Minecraft *minecraft, int x, int y, int width, int height);

typedef void (*misc_update_function_Minecraft_t)(Minecraft *obj);
void misc_run_on_update(misc_update_function_Minecraft_t function); // obj == Minecraft *
void misc_run_on_tick(misc_update_function_Minecraft_t function); // obj == Minecraft *
typedef void (*misc_update_function_Recipes_t)(Recipes *obj);
void misc_run_on_recipes_setup(misc_update_function_Recipes_t function); // obj == Recipes *
typedef void (*misc_update_function_FurnaceRecipes_t)(FurnaceRecipes *obj);
void misc_run_on_furnace_recipes_setup(misc_update_function_FurnaceRecipes_t function); // obj == FurnaceRecipes *
typedef void (*misc_update_function_FillingContainer_t)(FillingContainer *obj);
void misc_run_on_creative_inventory_setup(misc_update_function_FillingContainer_t function); // obj == FillingContainer *
typedef void (*misc_update_function_void_t)(void *obj);
void misc_run_on_tiles_setup(misc_update_function_void_t function); // obj == NULL
void misc_run_on_items_setup(misc_update_function_void_t function); // obj == NULL
void misc_run_on_language_setup(misc_update_function_void_t function); // obj == NULL
typedef bool (*misc_update_function_key_press_t)(Minecraft *minecrtaft, int key);
void misc_run_on_game_key_press(misc_update_function_key_press_t function); // In-Game Key Presses Only

extern bool is_in_chat;
}