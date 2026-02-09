#include <mods/init/init.h>
#include <mods/feature/feature.h>

#include "internal.h"

// Init
void init_multiplayer() {
    // Server List
    if (feature_has("External Server Support", server_disabled)) {
        _init_multiplayer_server_list();
    }

    // Init Other Fixes
    _init_multiplayer_raknet();
    _init_multiplayer_misc();
    _init_multiplayer_syncing();
    _init_multiplayer_inventory();

    // Allow Adding Extra Flags To StartGamePacket
    _init_multiplayer_start_game_flags();

    // Improved Chunk Loading
    if (feature_has("Improve Multiplayer Chunk Loading", server_enabled)) {
        _init_multiplayer_loading();
    }
}