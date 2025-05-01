#include <queue>

#include <libreborn/patch.h>
#include <mods/misc/misc.h>

#include "internal.h"

// Handle Disconnection While Loading
static bool should_disconnect = false;
static void ClientSideNetworkHandler_onDisconnect_injection(ClientSideNetworkHandler_onDisconnect_t original, ClientSideNetworkHandler *self, const RakNet_RakNetGUID &rak_net_guid) {
    // Call Original Method
    original(self, rak_net_guid);

    // Handle Stopping loading
    if (_multiplayer_is_loading_chunks(self)) {
        should_disconnect = true;
    }
}
static void handle_disconnect_tick(Minecraft *minecraft) {
    if (!should_disconnect) {
        return;
    }
    should_disconnect = false;

    // Stop Thread
    _multiplayer_stop_thread(minecraft);

    // Leave The Game
    minecraft->level_generation_signal = true;
    minecraft->leaveGame(false); // This Destroys Self!
    DisconnectionScreen *screen = DisconnectionScreen::allocate();
    screen->constructor(Strings::unable_to_connect);
    minecraft->setScreen((Screen *) screen);
}

// Buffered Block Updates
struct BlockUpdate {
    const int x;
    const int y;
    const int z;
    const int tile_id;
    const int data;
};
static std::queue<BlockUpdate> updates;
void _multiplayer_clear_updates() {
    while (!updates.empty()) {
        updates.pop();
    }
}
void _multiplayer_set_tile(int x, int y, int z, int tile_id, int data) {
    updates.emplace(x, y, z, tile_id, data);
}

// Handle Level Generation
static void ClientSideNetworkHandler_levelGenerated_injection(ClientSideNetworkHandler_levelGenerated_t original, ClientSideNetworkHandler *self, Level *level) {
    // Check If Using Improved Chunk Loading
    if (!_multiplayer_is_loading_chunks(self)) {
        // Call Original Method
        original(self, level);
        return;
    }

    // Set Level
    self->level = level;

    // Send Packets
    for (const uchar type : {1, 2}) {
        ReadyPacket *packet = ReadyPacket::allocate();
        ((Packet *) packet)->constructor();
        packet->vtable = ReadyPacket_vtable::base;
        packet->type = type;
        self->rak_net_instance->send(*(Packet *) packet);
        packet->destructor_deleting();
    }

    // Handle Buffered Block Updates
    while (!updates.empty()) {
        const BlockUpdate &update = updates.front();
        const int tile_id = Tile::transformToValidBlockId(update.tile_id, update.x, update.y, update.z);
        level->setTileAndData(update.x, update.y, update.z, tile_id, update.data);
        updates.pop();
    }
}

// Init
void _init_multiplayer_loading_misc() {
    // Handle Disconnection
    overwrite_calls(ClientSideNetworkHandler_onDisconnect, ClientSideNetworkHandler_onDisconnect_injection);
    misc_run_on_tick(handle_disconnect_tick);

    // Handle Level Generation
    overwrite_calls(ClientSideNetworkHandler_levelGenerated, ClientSideNetworkHandler_levelGenerated_injection);
}