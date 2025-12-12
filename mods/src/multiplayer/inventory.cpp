#include <libreborn/patch.h>

#include <symbols/ClientSideNetworkHandler.h>
#include <symbols/ContainerSetContentPacket.h>
#include <symbols/Minecraft.h>
#include <symbols/LocalPlayer.h>
#include <symbols/Inventory.h>
#include <symbols/SendInventoryPacket.h>
#include <symbols/Packet.h>
#include <symbols/Level.h>
#include <symbols/RakNetInstance.h>

#include <mods/multiplayer/packets.h>
#include <mods/feature/feature.h>
#include "internal.h"

// Receive Armor From Server
static void ClientSideNetworkHandler_handle_ContainerSetContentPacket_injection_armor(ClientSideNetworkHandler_handle_ContainerSetContentPacket_t original, ClientSideNetworkHandler *self, const RakNet_RakNetGUID &guid, ContainerSetContentPacket *packet) {
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

// Clear Inventory Before Receiving Inventory
static void ClientSideNetworkHandler_handle_ContainerSetContentPacket_injection_inventory(ClientSideNetworkHandler_handle_ContainerSetContentPacket_t original, ClientSideNetworkHandler *self, const RakNet_RakNetGUID &guid, ContainerSetContentPacket *packet) {
    // Clear Inventory
    if (packet->container_id == multiplayer_inventory_container_id && self->minecraft) {
        const LocalPlayer *player = self->minecraft->player;
        if (player) {
            player->inventory->dropAll(true);
        }
    }
    // Call Original Method
    original(self, guid, packet);
}
static void ClientSideNetworkHandler_handle_ContainerSetContentPacket_Inventory_setItem_injection(Inventory *self, const int slot, ItemInstance *item_instance) {
    // Don't Set Null Items
    if (item_instance && !item_instance->isNull()) {
        self->setItem(slot, item_instance);
    }
}

// Send Inventory
static void send_inventory(LocalPlayer *self) {
    // Not Needed In Creative
    Inventory *inventory = self->inventory;
    if (inventory->is_creative) {
        return;
    }

    // Create Packet
    SendInventoryPacket *packet = SendInventoryPacket::allocate();
    ((Packet *) packet)->constructor();
    packet->vtable = SendInventoryPacket::VTable::base;
    new (&packet->items) std::vector<ItemInstance>;

    // Configure Packet
    packet->entity_id = self->id;
    multiplayer_negate(packet->entity_id);
    // Inventory
    ItemInstance empty;
    empty.setNull();
    packet->inventory_size = 0;
    for (int i = inventory->linked_slots_length; i < inventory->getContainerSize(); i++) {
        packet->inventory_size++;
        const ItemInstance *item = inventory->getItem(i);
        packet->items.push_back(item ? *item : empty);
    }
    // Armor
    for (int i = 0; i < multiplayer_armor_size; i++) {
        const ItemInstance *item = self->getArmor(i);
        packet->items.push_back(item ? *item : empty);
    }

    // Send
    self->level->rak_net_instance->send(*(Packet *) packet);
    packet->destructor_deleting();
}
static void LocalPlayer_tick_injection(LocalPlayer_tick_t original, LocalPlayer *self) {
    // Call Original Method
    original(self);
    // Custom Behavior
    if (self->level->is_client_side) {
        // Check Time
        static int tick = 0;
        const bool should_send = tick == 0;
        tick++;
        constexpr int period = 10; // Half-A-Second
        if (tick >= period) {
            tick = 0;
        }
        if (!should_send) {
            return;
        }
        // Send Packet
        send_inventory(self);
    }
}
static void Minecraft_leaveGame_injection(Minecraft_leaveGame_t original, Minecraft *self, const bool param_1) {
    // Send Inventory On Disconnect
    if (self->isLevelGenerated() && self->level->is_client_side && self->player) {
        send_inventory(self->player);
    }
    // Call Original Method
    original(self, param_1);
}
static void ClientSideNetworkHandler_handle_SendInventoryPacket_injection(ClientSideNetworkHandler *self, MCPI_UNUSED const RakNet_RakNetGUID &guid, MCPI_UNUSED SendInventoryPacket *packet) {
    // Server Has Requested Inventory
    LocalPlayer *player = self->minecraft->player;
    if (player) {
        send_inventory(player);
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
        overwrite_calls(ClientSideNetworkHandler_handle_ContainerSetContentPacket, ClientSideNetworkHandler_handle_ContainerSetContentPacket_injection_armor);
    }

    // Receive Inventory
    if (feature_has("Correctly Handle Empty Slots When Receiving Inventory Data", server_disabled)) {
        overwrite_calls(ClientSideNetworkHandler_handle_ContainerSetContentPacket, ClientSideNetworkHandler_handle_ContainerSetContentPacket_injection_inventory);
        // Prevent Buggy Empty Slots
        // Listed As "EmptyItemInstance.name<"
        overwrite_call((void *) 0x6d3e8, Inventory_setItem, ClientSideNetworkHandler_handle_ContainerSetContentPacket_Inventory_setItem_injection);
    }

    // Send Inventory
    if (feature_has("Send Inventory To Server", server_disabled)) {
        overwrite_calls(LocalPlayer_tick, LocalPlayer_tick_injection);
        overwrite_calls(Minecraft_leaveGame, Minecraft_leaveGame_injection);
        patch_vtable(ClientSideNetworkHandler_handle_SendInventoryPacket, ClientSideNetworkHandler_handle_SendInventoryPacket_injection);
    }
}