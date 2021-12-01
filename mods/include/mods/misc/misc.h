#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t misc_get_real_selected_slot(unsigned char *player);

typedef void (*misc_update_function_t)(unsigned char *obj);
void misc_run_on_update(misc_update_function_t function); // obj == Minecraft *
void misc_run_on_tick(misc_update_function_t function); // obj == Minecraft *
void misc_run_on_recipes_setup(misc_update_function_t function); // obj == Recipes *
void misc_run_on_furnace_recipes_setup(misc_update_function_t function); // obj == FurnaceRecipes *
void misc_run_on_creative_inventory_setup(misc_update_function_t function); // obj == FillingContainer *
void misc_run_on_tiles_setup(misc_update_function_t function); // obj == NULL
void misc_run_on_internal_after_tiles_setup(misc_update_function_t function); // obj == NULL
void misc_run_on_items_setup(misc_update_function_t function); // obj == NULL

void Level_saveLevelData_injection(unsigned char *level);

// Use this instead of directly calling Gui::addMessage(), it has proper logging!
void misc_add_message(unsigned char *gui, const char *text);

#ifdef __cplusplus
}
#endif
