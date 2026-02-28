#pragma once

struct Screen;

MCPI_MODS_PUBLIC int get_seed_from_string(std::string str);
MCPI_MODS_PUBLIC Screen *game_mode_create_screen(bool creating_new_world = true, const std::string &old_world_name = "");
