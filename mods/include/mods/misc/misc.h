#pragma once

#include <cstdint>

#include <symbols/minecraft.h>

extern "C" {
int32_t misc_get_real_selected_slot(Player *player);
void misc_render_background(int color, Minecraft *minecraft, int x, int y, int width, int height);

typedef void (*misc_update_function_FillingContainer_t)(FillingContainer *obj);
void misc_run_on_creative_inventory_setup(misc_update_function_FillingContainer_t function); // obj == FillingContainer *

extern bool is_in_chat;
}

#define misc_run_on_update(...) \
    overwrite_virtual_calls(Minecraft_update, [](Minecraft_update_t _original, Minecraft *_self) { \
        _original(_self); \
        (__VA_ARGS__)(_self); \
    })
#define misc_run_on_tick(...) \
    overwrite_calls(Minecraft_tick, [](Minecraft_tick_t _original, Minecraft *_self, int _tick, int _max_ticks) { \
        _original(_self, _tick, _max_ticks); \
        (__VA_ARGS__)(_self); \
    })
#define misc_run_on_recipes_setup(...) \
    overwrite_calls(Recipes_constructor, [](Recipes_constructor_t _original, Recipes *_self) { \
        _original(_self); \
        (__VA_ARGS__)(_self); \
        return _self; \
    })
#define misc_run_on_furnace_recipes_setup(...) \
    overwrite_calls(FurnaceRecipes_constructor, [](FurnaceRecipes_constructor_t _original, FurnaceRecipes *_self) { \
        _original(_self); \
        (__VA_ARGS__)(_self); \
        return _self; \
    })
#define misc_run_on_tiles_setup(...) \
    overwrite_calls(Tile_initTiles, [](Tile_initTiles_t _original) { \
        (__VA_ARGS__)(); \
        _original(); \
    })
#define misc_run_on_items_setup(...) \
    overwrite_calls(Item_initItems, [](Item_initItems_t _original) { \
        _original(); \
        (__VA_ARGS__)(); \
    })
#define misc_run_on_language_setup(...) \
    overwrite_calls(I18n_loadLanguage, [](I18n_loadLanguage_t _original, AppPlatform *_self, std::string _language_name) { \
        _original(_self, _language_name); \
        (__VA_ARGS__)(); \
    })
#define misc_run_on_game_key_press(...) \
    overwrite_calls(Gui_handleKeyPressed, [](Gui_handleKeyPressed_t _original, Gui *_self, int _key) { \
        if ((__VA_ARGS__)(_self->minecraft, _key)) { \
            return; \
        } \
        _original(_self, _key); \
    })
