#pragma once

#include <cstdint>
#include <functional>

#include <symbols/minecraft.h>

extern "C" {
int32_t misc_get_real_selected_slot(const Player *player);
void misc_render_background(int color, const Minecraft *minecraft, int x, int y, int width, int height);

extern bool is_in_chat;

typedef RakNet_RakString *(*RakNet_RakString_constructor_2_t)(RakNet_RakString *self, const char *format, ...);
extern RakNet_RakString_constructor_2_t RakNet_RakString_constructor_2;
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
void misc_run_on_creative_inventory_setup(const std::function<void(FillingContainer *)> &function);
void misc_run_on_swap_buffers(const std::function<void()> &function);

static constexpr int line_height = 8;