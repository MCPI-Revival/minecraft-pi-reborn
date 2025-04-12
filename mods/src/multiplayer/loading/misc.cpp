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

    // Leave Game
    minecraft->level_generation_signal = true;
    minecraft->leaveGame(false); // This Destroys Self!
    DisconnectionScreen *screen = DisconnectionScreen::allocate();
    screen->constructor(Strings::unable_to_connect);
    minecraft->setScreen((Screen *) screen);
}

// Init
void _init_multiplayer_loading_misc() {
    // Handle Disconnection
    overwrite_calls(ClientSideNetworkHandler_onDisconnect, ClientSideNetworkHandler_onDisconnect_injection);
    misc_run_on_tick(handle_disconnect_tick);
}