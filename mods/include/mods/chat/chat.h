#pragma once

#include <symbols/minecraft.h>

extern "C" {
// Override using the HOOK() macro to provide customized chat behavior.
void ServerSideNetworkHandler_handle_ChatPacket_injection(ServerSideNetworkHandler *server_side_network_handler, const RakNet_RakNetGUID &rak_net_guid, ChatPacket *chat_packet);
void chat_send_message_to_clients(ServerSideNetworkHandler *server_side_network_handler, const Player *sender, const char *message);
void chat_handle_packet_send(const Minecraft *minecraft, ChatPacket *packet);
bool chat_is_sending();
}
