#pragma once

// Used To Designate Special Behavior
void multiplayer_negate(int &x);

// Armor Inventory Size
static constexpr int multiplayer_armor_size = 4;

// ContainerSetContentPacket IDs

// Used To Overwrite Inventory
static constexpr unsigned char multiplayer_inventory_container_id = 0; // This Is Also A Valid Container ID
// Used To Overwrite Armor
// This Is Added By Reborn
static constexpr unsigned char multiplayer_armor_container_id = 200;