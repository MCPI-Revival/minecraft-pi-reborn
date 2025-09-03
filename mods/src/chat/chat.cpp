#include <libreborn/patch.h>
#include <libreborn/util/string.h>

#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include <mods/chat/chat.h>
#include <mods/api/api.h>

#include "internal.h"

// Send API Chat Command
static void send_api_chat_command(const Minecraft *minecraft, const char *str) {
    // Construct Packet
    ChatPacket *packet = ChatPacket::allocate();
    ((Packet *) packet)->constructor();
    packet->vtable = ChatPacket::VTable::base;
    new (&packet->message) std::string;
    // Configure Packet
    packet->message = str;
    packet->param_1 = false;
    // Send Packet
    chat_handle_packet_send(minecraft, packet);
    // Destroy Packet
    packet->destructor_deleting();
}

// Send Message To Players
std::string _chat_get_prefix(const char *username) {
    return std::string("<") + username + "> ";
}
static bool is_sending = false;
bool chat_is_sending() {
    return is_sending;
}
void chat_send_message_to_clients(ServerSideNetworkHandler *server_side_network_handler, const Player *sender, const char *message) {
    is_sending = true;
    api_add_chat_event(sender, message);
    const char *username = ((Player *) sender)->username.c_str();
    std::string full_message = _chat_get_prefix(username) + message;
    sanitize_string(full_message, MAX_CHAT_MESSAGE_LENGTH, false);
    server_side_network_handler->displayGameMessage(full_message);
    is_sending = false;
}

// Only Send Messages To Fully Connected Players
static void ServerSideNetworkHandler_displayGameMessage_injection(MCPI_UNUSED ServerSideNetworkHandler_displayGameMessage_t original, ServerSideNetworkHandler *self, const std::string &message) {
    // Display Locally
    self->minecraft->gui.addMessage(message);
    // Create Packet
    MessagePacket *packet = MessagePacket::allocate();
    ((Packet *) packet)->constructor();
    packet->vtable = MessagePacket::VTable::base;
    packet->message.constructor();
    packet->message.Assign(message.c_str());
    packet->reliability = RELIABLE_ORDERED;
    // Send Packet
    const Level *level = self->level;
    if (level) {
        for (Player *player : self->level->players) {
            if (player->vtable == (const Player::VTable *) ServerPlayer::VTable::base) {
                const ServerPlayer *server_player = (ServerPlayer *) player;
                const RakNet_RakNetGUID &guid = server_player->guid;
                self->rak_net_instance->sendTo(guid, *(Packet *) packet);
            }
        }
    }
    packet->destructor_deleting();
}

// Handle Sending Chat Packet
void chat_handle_packet_send(const Minecraft *minecraft, ChatPacket *packet) {
    RakNetInstance *rak_net_instance = minecraft->rak_net_instance;
    if (rak_net_instance->isServer()) {
        // Handle Packet
        const char *message = packet->message.c_str();
        ServerSideNetworkHandler *server_side_network_handler = (ServerSideNetworkHandler *) minecraft->network_handler;
        chat_send_message_to_clients(server_side_network_handler, (Player *) minecraft->player, message);
    } else {
        // Pass Packet To Server
        rak_net_instance->send(*(Packet *) packet);
    }
}

// Handle ChatPacket Server-Side
void ServerSideNetworkHandler_handle_ChatPacket_injection(ServerSideNetworkHandler *server_side_network_handler, const RakNet_RakNetGUID &rak_net_guid, ChatPacket *chat_packet) {
    const Player *player = server_side_network_handler->getPlayer(rak_net_guid);
    if (player != nullptr) {
        const char *message = chat_packet->message.c_str();
        chat_send_message_to_clients(server_side_network_handler, player, message);
    }
}

// Send Message
void _chat_send_message_to_server(const Minecraft *minecraft, const char *message) {
    send_api_chat_command(minecraft, message);
}

// Allow Reading Longer ChatPacket Messages
static void ChatPacket_read_injection(MCPI_UNUSED ChatPacket_read_t original, ChatPacket *self, RakNet_BitStream *stream) {
    RakNet_RakString *str = RakNet_RakString::allocate();
    str->constructor();
    str->Deserialize(stream);
    self->message = str->sharedString->c_str;
    str->Free();
    ::operator delete(str);
}

// Clear Old Chat Messages When Joining Worlds
static void Minecraft_setLevel_injection(Minecraft_setLevel_t original, Minecraft *self, Level *level, const std::string &param_1, LocalPlayer *player) {
    // Clear
    self->gui.messages.clear();
    _chat_clear_history();
    // Call Original Method
    original(self, level, param_1, player);
}

// Init
void init_chat() {
    if (feature_has("Implement Chat", server_enabled)) {
        // Handle ChatPacket Server-Side
        patch_vtable(ServerSideNetworkHandler_handle_ChatPacket, ServerSideNetworkHandler_handle_ChatPacket_injection);
        // Init UI
        _init_chat_ui();
        // Disable Built-In Chat Message Limiting
        unsigned char message_limit_patch[4] = {0x03, 0x00, 0x53, 0xe1}; // "cmp r4, r4"
        patch((void *) 0x6b4c0, message_limit_patch);
        overwrite_calls(ChatPacket_read, ChatPacket_read_injection);
    }
    // Clear Chat Messages
    if (feature_has("Clear Old Chat Messages When Joining Worlds", server_enabled)) {
        overwrite_calls(Minecraft_setLevel, Minecraft_setLevel_injection);
    }
    // Only Send Messages To Fully Connected Players
    // This prevents unauthenticated players from
    // receiving chat messages.
    if (feature_has("Only Send Messages To Fully Connected Players", server_enabled)) {
        overwrite_calls(ServerSideNetworkHandler_displayGameMessage, ServerSideNetworkHandler_displayGameMessage_injection);
    }
}
