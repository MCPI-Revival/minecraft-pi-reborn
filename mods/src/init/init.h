#ifndef INIT_H

#define INIT_H

#ifdef __cplusplus
extern "C" {
#endif

void init_compat();
void init_server();
void init_game_mode();
void init_input();
void init_misc();
void init_camera();
void init_options();
void init_textures();

#ifdef __cplusplus
}
#endif

#endif