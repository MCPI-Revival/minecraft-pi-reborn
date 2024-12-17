#include <libreborn/patch.h>
#include <libreborn/util/string.h>

#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include "chat-internal.h"
#include <mods/chat/chat.h>

// Send UTF-8 API Command
std::string chat_send_api_command(const Minecraft *minecraft, const std::string &str) {
    ConnectedClient client;
    client.sock = -1;
    client.str = "";
    client.time = 0;
    CommandServer *command_server = minecraft->command_server;
    if (command_server != nullptr) {
        return command_server->parse(client, str);
    } else {
        return "";
    }
}

// Send API Chat Command
static void send_api_chat_command(const Minecraft *minecraft, const char *str) {
    const std::string utf_str = from_cp437(str);
    const std::string command = std::string("chat.post(") + utf_str + ")\n";
    chat_send_api_command(minecraft, command);
}

// Send Message To Players
std::string _chat_get_prefix(const char *username) {
    return std::string("<") + username + "> ";
}
void chat_send_message_to_clients(ServerSideNetworkHandler *server_side_network_handler, const char *username, const char *message) {
    std::string full_message = _chat_get_prefix(username) + message;
    sanitize_string(full_message, MAX_CHAT_MESSAGE_LENGTH, false);
    server_side_network_handler->displayGameMessage(full_message);
}
// Handle Chat packet Send
void chat_handle_packet_send(const Minecraft *minecraft, ChatPacket *packet) {
    // Convert To CP-437
    packet->message = to_cp437(packet->message);
    // Send
    RakNetInstance *rak_net_instance = minecraft->rak_net_instance;
    if (rak_net_instance->isServer()) {
        // Hosting Multiplayer
        const char *message = packet->message.c_str();
        ServerSideNetworkHandler *server_side_network_handler = (ServerSideNetworkHandler *) minecraft->network_handler;
        chat_send_message_to_clients(server_side_network_handler, Strings::default_username, (char *) message);
    } else {
        // Client
        rak_net_instance->send(*(Packet *) packet);
    }
}

// Manually Send (And Loopback) ChatPacket
static void CommandServer_parse_CommandServer_dispatchPacket_injection(CommandServer *command_server, Packet &packet) {
    const Minecraft *minecraft = command_server->minecraft;
    if (minecraft != nullptr) {
        chat_handle_packet_send(minecraft, (ChatPacket *) &packet);
    }
}

// Handle ChatPacket Server-Side
static void ServerSideNetworkHandler_handle_ChatPacket_injection(ServerSideNetworkHandler *server_side_network_handler, const RakNet_RakNetGUID &rak_net_guid, ChatPacket *chat_packet) {
    const Player *player = server_side_network_handler->getPlayer(rak_net_guid);
    if (player != nullptr) {
        const char *username = player->username.c_str();
        const char *message = chat_packet->message.c_str();
        chat_send_message_to_clients(server_side_network_handler, username, message);
    }
}

// Send Message
void _chat_send_message_to_server(const Minecraft *minecraft, const char *message) {
    send_api_chat_command(minecraft, message);
}

// Allow Reading Longer ChatPacket Messages
static void ChatPacket_read_injection(__attribute__((unused)) ChatPacket_read_t original, ChatPacket *self, RakNet_BitStream *stream) {
    RakNet_RakString *str = RakNet_RakString::allocate();
    str->constructor();
    str->Deserialize(stream);
    self->message = str->sharedString->c_str;
    str->Free();
    ::operator delete(str);
}

// Init
void init_chat() {
    if (feature_has("Implement Chat", server_enabled)) {
        // Disable Original ChatPacket Loopback
        unsigned char disable_chat_packet_loopback_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x6b490, disable_chat_packet_loopback_patch);
        // Manually Send (And Loopback) ChatPacket
        overwrite_call((void *) 0x6b518, CommandServer_dispatchPacket, CommandServer_parse_CommandServer_dispatchPacket_injection);
        // Re-Broadcast ChatPacket
        patch_vtable(ServerSideNetworkHandler_handle_ChatPacket, ServerSideNetworkHandler_handle_ChatPacket_injection);
        // Init UI
        _init_chat_ui();
        // Disable Built-In Chat Message Limiting
        unsigned char message_limit_patch[4] = {0x03, 0x00, 0x53, 0xe1}; // "cmp r4, r4"
        patch((void *) 0x6b4c0, message_limit_patch);
        overwrite_calls(ChatPacket_read, ChatPacket_read_injection);
    }
}
