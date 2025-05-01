#include <libreborn/patch.h>

#include <mods/multiplayer/packets.h>
#include <mods/feature/feature.h>
#include "internal.h"

// Init
void _init_multiplayer_inventory() {
    // Fix Opening Containers
    if (feature_has("Fix Containers Overwriting The Inventory", server_enabled)) {
        constexpr unsigned char new_min_id = multiplayer_inventory_container_id + 1;
        unsigned char initial_id_patch[4] = {new_min_id, 0x30, 0xa0, 0xe3}; // "mov r3, #new_min_id"
        patch((void *) 0x77188, initial_id_patch);
        unsigned char wraparound_id_patch[4] = {new_min_id, 0x30, 0xa0, 0xc3}; // "movgt r3, #new_min_id"
        patch((void *) 0x771c0, wraparound_id_patch);
    }
}

// send inventory every 10 ticks or when leaving game

// allow receiving armor data from server

// use negated entity ID so items don't drop
// - make negation a global function?

// Add SetSpawnPositionPacket to syncing.cpp under "Sync Level Spawn Position"

// Only save player-data if inventory dats is present