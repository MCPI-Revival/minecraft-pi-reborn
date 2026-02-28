#pragma once

#include <string>

struct Player;
struct Entity;
struct RakNet_RakNetGUID;

extern "C" {
MCPI_MODS_PUBLIC void api_add_chat_event(const Player *sender, const std::string &message);
MCPI_MODS_PUBLIC void api_update_entity_position(const Entity *entity, const RakNet_RakNetGUID *guid = nullptr);
}