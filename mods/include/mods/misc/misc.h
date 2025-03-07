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

void misc_set_on_fire(Mob *mob, int seconds);

void misc_run_on_init(const std::function<void(Minecraft *)> &func);
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

std::string misc_get_player_username_utf(const Player *player);

enum class EntityType {
    UNKNOWN = 0,
    // Animals
    CHICKEN = 10,
    COW = 11,
    PIG = 12,
    SHEEP = 13,
    // Hostiles
    ZOMBIE = 32,
    CREEPER = 33,
    SKELETON = 34,
    SPIDER = 35,
    ZOMBIE_PIGMAN = 36,
    // Special #1, Miscellaneous
    DROPPED_ITEM = 64,
    PRIMED_TNT = 65,
    FALLING_SAND = 66,
    PAINTING = 83,
    // Special #2, Projectiles
    ARROW = 80,
    THROWN_SNOWBALL = 81,
    THROWN_EGG = 82
};
std::map<EntityType, std::pair<std::string, std::string>> &misc_get_entity_type_names();
std::pair<std::string, std::string> misc_get_entity_type_name(Entity *entity);
std::string misc_get_entity_name(Entity *entity);

Entity *misc_make_entity_from_id(Level *level, int id, float x, float y, float z, bool add_to_level = true);

std::string misc_base64_encode(const std::string &data);
std::string misc_base64_decode(const std::string &input);

static constexpr int line_height = 8;