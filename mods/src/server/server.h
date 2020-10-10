#ifndef SERVER_H

#define SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

void server_init();

const char *server_get_motd();
int server_get_default_game_mode();
int server_get_mob_spawning();

#ifdef __cplusplus
}
#endif

#endif
