#ifndef SERVER_INTERNAL_H

#define SERVER_INTERNAL_H

#include <string>
#include <vector>

std::string server_internal_get_world_name();
unsigned char *server_internal_get_level(unsigned char *minecraft);
std::vector<unsigned char *> server_internal_get_players(unsigned char *level);
std::string server_internal_get_server_player_username(unsigned char *player);
unsigned char *server_internal_get_minecraft(unsigned char *player);

#endif