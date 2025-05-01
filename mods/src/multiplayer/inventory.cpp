#include <libreborn/patch.h>
#include <symbols/minecraft.h>

#include <mods/multiplayer/packets.h>
#include <mods/feature/feature.h>
#include "internal.h"

// Receive Armor From Server
static void ClientSideNetworkHandler_handle_ContainerSetContentPacket_injection(ClientSideNetworkHandler_handle_ContainerSetContentPacket_t original, ClientSideNetworkHandler *self, const RakNet_RakNetGUID &guid, ContainerSetContentPacket *packet) {
    if (packet->container_id == multiplayer_armor_container_id) {
        // Custom Behavior
        if (!self->minecraft) {
            return;
        }
        LocalPlayer *player = self->minecraft->player;
        if (!player) {
            return;
        }
        const int size = std::min(int(packet->items.size()), multiplayer_armor_size);
        for (int i = 0; i < size; i++) {
            ItemInstance *item = &packet->items[i];
            if (item->isNull()) {
                item = nullptr;
            }
            player->setArmor(i, item);
        }
    } else {
        // Call Original Method
        original(self, guid, packet);
    }
}

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

    // Armor
    if (feature_has("Allow Servers To Overwrite Armor", server_disabled)) {
        overwrite_calls(ClientSideNetworkHandler_handle_ContainerSetContentPacket, ClientSideNetworkHandler_handle_ContainerSetContentPacket_injection);
    }
}

// send inventory every 10 ticks or when leaving game

// allow receiving armor data from server

// use negated entity ID so items don't drop
// - make negation a global function?

// Add SetSpawnPositionPacket to syncing.cpp under "Sync Level Spawn Position"

// Only save player-data if inventory dats is present