#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void run_tests();
void init_compat();
#ifdef MCPI_SERVER_MODE
void init_server();
#endif
void init_game_mode();
void init_input();
void init_misc();
void init_death();
void init_camera();
void init_options();
void init_touch();
void init_textures();
void init_chat();
void init_home();

#ifdef __cplusplus
}
#endif
