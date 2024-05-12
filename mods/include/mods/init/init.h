#pragma once

extern "C" {
void run_tests();
void init_version();
void init_compat();
#ifdef MCPI_SERVER_MODE
void init_server();
#else
void init_multiplayer();
void init_benchmark();
#endif
#ifndef MCPI_HEADLESS_MODE
void init_sound();
void init_input();
void init_sign();
void init_camera();
void init_atlas();
void init_title_screen();
void init_skin();
void init_fps();
#endif
void init_touch();
void init_textures();
void init_creative();
void init_game_mode();
void init_misc();
void init_death();
void init_options();
void init_chat();
void init_bucket();
void init_cake();
void init_home();
}
