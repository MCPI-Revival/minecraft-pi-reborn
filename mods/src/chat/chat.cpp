// Config Needs To Load First
#include <libreborn/libreborn.h>

#include <string>
#include <cstring>
#include <cstdio>
#include <vector>
#ifndef MCPI_SERVER_MODE
#include <pthread.h>
#endif

#include <symbols/minecraft.h>
#ifndef MCPI_SERVER_MODE
#include <media-layer/core.h>
#endif

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#ifndef MCPI_SERVER_MODE
#include <mods/input/input.h>
#endif
#include "chat-internal.h"
#include <mods/chat/chat.h>

// Store If Chat is Enabled
int _chat_enabled = 0;

// Message Limitations
#define MAX_CHAT_MESSAGE_LENGTH 512

// Send API Command
std::string chat_send_api_command(unsigned char *minecraft, char *str) {
    struct ConnectedClient client;
    client.sock = -1;
    client.str = "";
    client.time = 0;
    unsigned char *command_server = *(unsigned char **) (minecraft + Minecraft_command_server_property_offset);
    if (command_server != NULL) {
        return (*CommandServer_parse)(command_server, client, str);
    } else {
        return "";
    }
}

#ifndef MCPI_SERVER_MODE
// Send API Chat Command
static void send_api_chat_command(unsigned char *minecraft, char *str) {
    char *command = NULL;
    safe_asprintf(&command, "chat.post(%s)\n", str);
    chat_send_api_command(minecraft, command);
    free(command);
}
#endif

// Send Message To Players
void chat_send_message(unsigned char *server_side_network_handler, char *username, char *message) {
    char *full_message = NULL;
    safe_asprintf(&full_message, "<%s> %s", username, message);
    sanitize_string(&full_message, MAX_CHAT_MESSAGE_LENGTH, 0);
    (*ServerSideNetworkHandler_displayGameMessage)(server_side_network_handler, std::string(full_message));
    free(full_message);
}
// Handle Chat packet Send
void chat_handle_packet_send(unsigned char *minecraft, unsigned char *packet) {
    unsigned char *rak_net_instance = *(unsigned char **) (minecraft + Minecraft_rak_net_instance_property_offset);
    unsigned char *rak_net_instance_vtable = *(unsigned char **) rak_net_instance;
    RakNetInstance_isServer_t RakNetInstance_isServer = *(RakNetInstance_isServer_t *) (rak_net_instance_vtable + RakNetInstance_isServer_vtable_offset);
    if ((*RakNetInstance_isServer)(rak_net_instance)) {
        // Hosting Multiplayer
        char *message = *(char **) (packet + ChatPacket_message_property_offset);
        unsigned char *server_side_network_handler = *(unsigned char **) (minecraft + Minecraft_network_handler_property_offset);
        chat_send_message(server_side_network_handler, *default_username, message);
    } else {
        // Client
        RakNetInstance_send_t RakNetInstance_send = *(RakNetInstance_send_t *) (rak_net_instance_vtable + RakNetInstance_send_vtable_offset);
        (*RakNetInstance_send)(rak_net_instance, packet);
    }
}

// Manually Send (And Loopback) ChatPacket
static void CommandServer_parse_CommandServer_dispatchPacket_injection(unsigned char *command_server, unsigned char *packet) {
    unsigned char *minecraft = *(unsigned char **) (command_server + CommandServer_minecraft_property_offset);
    if (minecraft != NULL) {
        chat_handle_packet_send(minecraft, packet);
    }
}

// Handle ChatPacket Server-Side
static void ServerSideNetworkHandler_handle_ChatPacket_injection(unsigned char *server_side_network_handler, RakNet_RakNetGUID *rak_net_guid, unsigned char *chat_packet) {
    unsigned char *level = *(unsigned char **) (server_side_network_handler + ServerSideNetworkHandler_level_property_offset);
    unsigned char *player = (*NetEventCallback_findPlayer)(server_side_network_handler, level, rak_net_guid);
    if (player != NULL) {
        char *username = *(char **) (player + Player_username_property_offset);
        char *message = *(char **) (chat_packet + ChatPacket_message_property_offset);
        chat_send_message(server_side_network_handler, username, message);
    }
}

#ifndef MCPI_SERVER_MODE
// Message Queue
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static std::vector<std::string> queue;
// Add To Queue
void _chat_queue_message(char *message) {
    // Lock
    pthread_mutex_lock(&queue_mutex);
    // Add
    std::string str;
    str.append(message);
    queue.push_back(str);
    // Unlock
    pthread_mutex_unlock(&queue_mutex);
}
// Empty Queue
unsigned int old_chat_counter = 0;
static void send_queued_messages(unsigned char *minecraft) {
    // Lock
    pthread_mutex_lock(&queue_mutex);
    // If Message Was Submitted, No Other Chat Windows Are Open, And The Game Is Not Paused, Then Re-Lock Cursor
    unsigned int new_chat_counter = chat_get_counter();
    if (old_chat_counter > new_chat_counter && new_chat_counter == 0) {
        // Unlock UI
        media_set_interactable(1);
    }
    old_chat_counter = new_chat_counter;
    // Loop
    for (unsigned int i = 0; i < queue.size(); i++) {
        send_api_chat_command(minecraft, (char *) queue[i].c_str());
    }
    queue.clear();
    // Unlock
    pthread_mutex_unlock(&queue_mutex);
}
#endif

// Init
void init_chat() {
    _chat_enabled = feature_has("Implement Chat", server_enabled);
    if (_chat_enabled) {
        // Disable Original ChatPacket Loopback
        unsigned char disable_chat_packet_loopback_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x8c118, disable_chat_packet_loopback_patch);
        // Manually Send (And Loopback) ChatPacket
        overwrite_call((void *) 0x8c1a4, (void *) CommandServer_parse_CommandServer_dispatchPacket_injection);
        // Re-Broadcast ChatPacket
        patch_address(ServerSideNetworkHandler_handle_ChatPacket_vtable_addr, (void *) ServerSideNetworkHandler_handle_ChatPacket_injection);
        // Send Messages On Input Tick
#ifndef MCPI_SERVER_MODE
        input_run_on_tick(send_queued_messages);
#endif
    }
}
