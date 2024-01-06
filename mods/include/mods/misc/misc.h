#pragma once

#include <stdint.h>

#include <symbols/minecraft.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t misc_get_real_selected_slot(Player *player);

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

void Level_saveLevelData_injection(Level *level);

// Use this instead of directly calling Gui::addMessage(), it has proper logging!
void misc_add_message(Gui *gui, const char *text);

#ifdef __cplusplus
}
#endif
