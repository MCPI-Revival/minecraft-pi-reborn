#pragma once

#include <symbols/minecraft.h>

int get_seed_from_string(std::string str);
Screen *game_mode_create_screen(bool creating_new_world = true, const std::string &old_world_name = "");
