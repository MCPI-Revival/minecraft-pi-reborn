#pragma once

#include <string>
#include <symbols/minecraft.h>

extern "C" {
void api_add_chat_event(const Player *sender, const std::string &message);
void api_update_entity_position(const Entity *entity, const RakNet_RakNetGUID *guid = nullptr);
}