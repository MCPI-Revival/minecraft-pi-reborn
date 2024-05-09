#include <string>
#include <cstring>
#include <cstdio>
#include <vector>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include "chat-internal.h"
#include <mods/chat/chat.h>

// Send API Command
std::string chat_send_api_command(Minecraft *minecraft, std::string str) {
    ConnectedClient client;
    client.sock = -1;
    client.str = "";
    client.time = 0;
    CommandServer *command_server = minecraft->command_server;
    if (command_server != nullptr) {
        return CommandServer_parse(command_server, &client, &str);
    } else {
        return "";
    }
}

// Send API Chat Command
static void send_api_chat_command(Minecraft *minecraft, char *str) {
    char *command = nullptr;
    safe_asprintf(&command, "chat.post(%s)\n", str);
    chat_send_api_command(minecraft, command);
    free(command);
}

// Send Message To Players
std::string _chat_get_prefix(char *username) {
    return std::string("<") + username + "> ";
}
void chat_send_message(ServerSideNetworkHandler *server_side_network_handler, char *username, char *message) {
    char *full_message = nullptr;
    safe_asprintf(&full_message, "%s%s", _chat_get_prefix(username).c_str(), message);
    sanitize_string(&full_message, MAX_CHAT_MESSAGE_LENGTH, 0);
    std::string cpp_string = full_message;
    free(full_message);
    ServerSideNetworkHandler_displayGameMessage(server_side_network_handler, &cpp_string);
}
// Handle Chat packet Send
void chat_handle_packet_send(Minecraft *minecraft, ChatPacket *packet) {
    RakNetInstance *rak_net_instance = minecraft->rak_net_instance;
    if (rak_net_instance->vtable->isServer(rak_net_instance)) {
        // Hosting Multiplayer
        const char *message = packet->message.c_str();
        ServerSideNetworkHandler *server_side_network_handler = (ServerSideNetworkHandler *) minecraft->network_handler;
        chat_send_message(server_side_network_handler, Strings_default_username, (char *) message);
    } else {
        // Client
        rak_net_instance->vtable->send(rak_net_instance, (Packet *) packet);
    }
}

// Manually Send (And Loopback) ChatPacket
static void CommandServer_parse_CommandServer_dispatchPacket_injection(CommandServer *command_server, Packet *packet) {
    Minecraft *minecraft = command_server->minecraft;
    if (minecraft != nullptr) {
        chat_handle_packet_send(minecraft, (ChatPacket *) packet);
    }
}

// Handle ChatPacket Server-Side
static void ServerSideNetworkHandler_handle_ChatPacket_injection(ServerSideNetworkHandler *server_side_network_handler, RakNet_RakNetGUID *rak_net_guid, ChatPacket *chat_packet) {
    Player *player = ServerSideNetworkHandler_getPlayer(server_side_network_handler, rak_net_guid);
    if (player != nullptr) {
        const char *username = player->username.c_str();
        const char *message = chat_packet->message.c_str();
        chat_send_message(server_side_network_handler, (char *) username, (char *) message);
    }
}

// Send Message
void _chat_send_message(Minecraft *minecraft, const char *message) {
    send_api_chat_command(minecraft, (char *) message);
}

// Init
void init_chat() {
    if (feature_has("Implement Chat", server_enabled)) {
        // Disable Original ChatPacket Loopback
        unsigned char disable_chat_packet_loopback_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x6b490, disable_chat_packet_loopback_patch);
        // Manually Send (And Loopback) ChatPacket
        overwrite_call((void *) 0x6b518, (void *) CommandServer_parse_CommandServer_dispatchPacket_injection);
        // Re-Broadcast ChatPacket
        patch_vtable(ServerSideNetworkHandler_handle_ChatPacket, ServerSideNetworkHandler_handle_ChatPacket_injection);
        // Init UI
        _init_chat_ui();
        // Disable Built-In Chat Message Limiting
        unsigned char message_limit_patch[4] = {0x03, 0x00, 0x53, 0xe1}; // "cmp r4, r4"
        patch((void *) 0x6b4c0, message_limit_patch);
    }
}
