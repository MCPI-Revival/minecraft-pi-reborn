#pragma once

struct ServerSideNetworkHandler;
struct RakNet_RakNetGUID;
struct ChatPacket;
struct Player;
struct Minecraft;

extern "C" {
// Override using the HOOK() macro to provide customized chat behavior.
MCPI_MODS_PUBLIC void ServerSideNetworkHandler_handle_ChatPacket_injection(ServerSideNetworkHandler *server_side_network_handler, const RakNet_RakNetGUID &rak_net_guid, ChatPacket *chat_packet);
MCPI_MODS_PUBLIC void chat_send_message_to_clients(ServerSideNetworkHandler *server_side_network_handler, const Player *sender, const char *message);
MCPI_MODS_PUBLIC void chat_handle_packet_send(const Minecraft *minecraft, ChatPacket *packet);
MCPI_MODS_PUBLIC bool chat_is_sending();
}
