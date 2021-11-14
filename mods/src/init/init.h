#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void run_tests();
void init_version();
void init_compat();
#ifdef MCPI_SERVER_MODE
void init_server();
#else
void init_multiplayer();
void init_sound();
void init_input();
void init_sign();
void init_creative();
void init_camera();
void init_touch();
void init_textures();
void init_atlas();
#endif
void init_game_mode();
void init_misc();
void init_death();
void init_options();
void init_chat();
void init_home();
#if !defined(MCPI_SERVER_MODE) && !defined(MCPI_HEADLESS_MODE)
void init_benchmark();
#endif

#ifdef __cplusplus
}
#endif
