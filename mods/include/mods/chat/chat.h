#pragma once

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#ifdef __cplusplus
#include <string>
// Send API Command
std::string chat_send_api_command(Minecraft *minecraft, std::string str);
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MCPI_SERVER_MODE
void chat_open();
unsigned int chat_get_counter();
#endif

// Override using the HOOK() macro to provide customized chat behavior.
void chat_send_message(ServerSideNetworkHandler *server_side_network_handler, char *username, char *message);
void chat_handle_packet_send(Minecraft *minecraft, ChatPacket *packet);

#ifdef __cplusplus
}
#endif
