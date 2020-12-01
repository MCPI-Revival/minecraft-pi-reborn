#ifndef SERVER_H

#define SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

void server_init();

const char *server_get_motd();

const char *server_get_features();

#ifdef __cplusplus
}
#endif

#endif
