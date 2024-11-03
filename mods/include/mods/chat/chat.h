#pragma once

#include <string>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

extern "C" {
// Send API Command
std::string chat_send_api_command(const Minecraft *minecraft, const std::string &str);

// Override using the HOOK() macro to provide customized chat behavior.
void chat_send_message_to_clients(ServerSideNetworkHandler *server_side_network_handler, const char *username, const char *message);
void chat_handle_packet_send(const Minecraft *minecraft, ChatPacket *packet);
}
