#pragma once

// Used To Designate Special Behavior
void multiplayer_negate(int &x);

// Armor Inventory Size
static constexpr int multiplayer_armor_size = 4;

// ContainerSetContentPacket IDs
//     Used To Overwrite Inventory
static constexpr unsigned char multiplayer_inventory_container_id = 0; // This Is Also A Valid Container ID
//     Used To Overwrite Armor
//     This Is Added By Reborn
static constexpr unsigned char multiplayer_armor_container_id = 200;

// Extra StartGamePacket Flags
namespace StartGameFlags {
    enum {
        USE_IMPROVED_LOADING = 0,
        FORCE_DAYNIGHT_CYCLE,
        FORCE_NO_DAYNIGHT_CYCLE
    };
    // Receiving Flags
    // This is UB if a StartGamePacket has not been received.
    bool has_received_from_server(int flag);
    // Sending Flags
    void send_to_clients(int flag);
    bool will_send_to_clients(int flag);
}