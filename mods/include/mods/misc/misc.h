#pragma once

#include <cstdint>
#include <functional>

#include <symbols/minecraft.h>

extern "C" {
int32_t misc_get_real_selected_slot(Player *player);
void misc_render_background(int color, Minecraft *minecraft, int x, int y, int width, int height);

typedef void (*misc_update_function_FillingContainer_t)(FillingContainer *obj);
void misc_run_on_creative_inventory_setup(misc_update_function_FillingContainer_t function); // obj == FillingContainer *

extern bool is_in_chat;
}

void misc_run_on_update(const std::function<void(Minecraft *)> &func);
void misc_run_on_tick(const std::function<void(Minecraft *)> &func);
void misc_run_on_recipes_setup(const std::function<void(Recipes *)> &func);
void misc_run_on_furnace_recipes_setup(const std::function<void(FurnaceRecipes *)> &func);
void misc_run_on_tiles_setup(const std::function<void()> &func);
void misc_run_on_items_setup(const std::function<void()> &func);
void misc_run_on_language_setup(const std::function<void()> &func);
void misc_run_on_game_key_press(const std::function<bool(Minecraft *, int)> &func);
void misc_run_on_key_press(const std::function<bool(Minecraft *, int)> &func);
