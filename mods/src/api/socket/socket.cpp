#include <sys/socket.h>
#include <unistd.h>

#include <libreborn/patch.h>

#include <mods/feature/feature.h>

#include "internal.h"

// Remove Speed Limit
static bool CommandServer__updateClient_injection_1(MCPI_UNUSED CommandServer__updateClient_t original, CommandServer *self, ConnectedClient &client) {
    return api_read_and_handle_data(self, client);
}

// Clear Extra Data On Socket Close
static bool CommandServer__updateClient_injection_2(CommandServer__updateClient_t original, CommandServer *self, ConnectedClient &client) {
    const bool ret = original(self, client);
    if (!ret) {
        // Client Disconnected
        api_free_event_data(client.sock);
        api_clear_write_queue(client.sock);
    }
    return ret;
}
static void CommandServer__close_injection_1(CommandServer__close_t original, CommandServer *self) {
    // Clear All Extra Data
    api_free_all_event_data();
    api_clear_write_queue();
    // Call Original Method
    original(self);
}

// Close Sockets
static void CommandServer__close_injection_2(CommandServer__close_t original, CommandServer *self) {
    // Close
    for (const ConnectedClient &client : self->clients) {
        close(client.sock);
    }
    self->clients.clear();
    // Call Original Method
    original(self);
}
static void Minecraft_leaveGame_injection(Minecraft_leaveGame_t original, Minecraft *self, const bool save_remote_level) {
    // Destroy Server
    CommandServer *&server = self->command_server;
    if (server) {
        server->destructor(0);
        ::operator delete(server);
        server = nullptr;
    }
    // Call Original Method
    original(self, save_remote_level);
}
static bool CommandServer__updateClient_injection_3(CommandServer__updateClient_t original, CommandServer *self, ConnectedClient &client) {
    // Call Original Method
    const bool ret = original(self, client);
    // Close Socket If Needed
    if (!ret) {
        close(client.sock);
    }
    return ret;
}

// Init
void _init_api_socket() {
    // Optimization
    if (feature_has("Optimize API Sockets", server_enabled)) {
        overwrite_calls(CommandServer__updateClient, CommandServer__updateClient_injection_1);
    }

    // Clear Extra Data On Socket Close
    overwrite_calls(CommandServer__updateClient, CommandServer__updateClient_injection_2);
    overwrite_calls(CommandServer__close, CommandServer__close_injection_1);

    // Correctly Close Sockets
    if (feature_has("Correctly Close API Sockets", server_enabled)) {
        overwrite_calls(CommandServer__close, CommandServer__close_injection_2);
        overwrite_calls(Minecraft_leaveGame, Minecraft_leaveGame_injection);
        overwrite_calls(CommandServer__updateClient, CommandServer__updateClient_injection_3);
    }
}