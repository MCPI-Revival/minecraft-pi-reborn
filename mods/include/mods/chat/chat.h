#pragma once

#include <string>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

extern "C" {
// Send API Command
std::string chat_send_api_command(Minecraft *minecraft, std::string str);

// Override using the HOOK() macro to provide customized chat behavior.
void chat_send_message(ServerSideNetworkHandler *server_side_network_handler, char *username, char *message);
void chat_handle_packet_send(Minecraft *minecraft, ChatPacket *packet);
};
