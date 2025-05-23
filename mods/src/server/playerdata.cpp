#include <fstream>
#include <vector>
#include <array>
#include <unordered_map>

#include <libreborn/patch.h>
#include <libreborn/util/util.h>

#include <mods/misc/misc.h>
#include <mods/api/api.h>
#include <mods/multiplayer/packets.h>
#include <mods/server/server.h>

#include "internal.h"

// Player Data
struct PlayerData {
    // Position
    float x;
    float y;
    float z;
    // Rotation
    float yaw;
    float pitch;
    // Spawn-Point
    bool has_spawn;
    int spawn_x;
    int spawn_y;
    int spawn_z;
    // Miscellaneous
    int health;
    int fire_timer;
    // Inventory
    int inventory_size;
};

// Track Inventories
struct InventoryData {
    std::vector<ItemInstance> inv;
    std::array<ItemInstance, multiplayer_armor_size> armor;
};
static std::unordered_map<int, InventoryData> inventories;

// Handle Client Inventory
static void ServerSideNetworkHandler_handle_SendInventoryPacket_injection(ServerSideNetworkHandler_handle_SendInventoryPacket_t original, ServerSideNetworkHandler *self, const RakNet_RakNetGUID &guid, SendInventoryPacket *packet) {
    if (packet->entity_id >= 0) {
        // Call Original Method
        original(self, guid, packet);
        return;
    }

    // Record Data
    if (!self->level) {
        return;
    }
    multiplayer_negate(packet->entity_id);
    const ServerPlayer *player = (ServerPlayer *) self->getPlayer(guid);
    if (!player || player->id != packet->entity_id) {
        return;
    }
    InventoryData data;
    int i;
    for (i = 0; i < packet->inventory_size; i++) {
        data.inv.push_back(packet->items[i]);
    }
    for (int j = 0; j < multiplayer_armor_size; j++) {
        data.armor[j] = packet->items[i + j];
    }
    inventories[player->id] = data;
}

// Get Data Path
static std::string get_data_file(const ServerPlayer *player) {
    // Get Directory
    const Level *level = player->level;
    const ExternalFileLevelStorage *storage = (ExternalFileLevelStorage *) level->storage;
    const std::string &path = storage->path + "/players";
    ensure_directory(path.c_str());
    // Get Path
    const std::string username = misc_base64_encode(player->username);
    return path + '/' + username + ".dat";
}

// Load Player Data
template <typename T>
static void safe_read(std::ifstream &file, T &obj, const bool required = true) {
    file.read((char *) &obj, sizeof(obj));
    if (!file && required) {
        ERR("Unable To Read Player Data");
    }
}
void _load_playerdata(ServerPlayer *player) {
    // Open File
    std::ifstream file(get_data_file(player), std::ios::binary);
    if (!file) {
        return;
    }

    // Read Data
    PlayerData data = {};
    safe_read(file, data);
    player->moveTo(data.x, data.y - player->height_offset, data.z, data.yaw, data.pitch);
    api_update_entity_position((Entity *) player, &player->guid);
    player->health = data.health;
    player->fire_timer = data.fire_timer;
    if (data.has_spawn) {
        const Pos spawn_pos = {
            .x = data.spawn_x,
            .y = data.spawn_y,
            .z = data.spawn_z
        };
        player->setRespawnPosition(spawn_pos);
    }

    // Inventory
    InventoryData inv_data;
    if (!player->inventory->is_creative) {
        // Create Packet
        ContainerSetContentPacket *packet = ContainerSetContentPacket::allocate();
        ((Packet *) packet)->constructor();
        packet->vtable = ContainerSetContentPacket::VTable::base;
        new (&packet->items) std::vector<ItemInstance>;

        // Send Inventory
        packet->container_id = multiplayer_inventory_container_id;
        for (int i = 0; i < data.inventory_size; i++) {
            ItemInstance item;
            safe_read(file, item);
            packet->items.push_back(item);
            inv_data.inv.push_back(item);
        }
        RakNetInstance *rak_net_instance = player->level->rak_net_instance;
        rak_net_instance->sendTo(player->guid, *(Packet *) packet);

        // Send Armor
        packet->container_id = multiplayer_armor_container_id;
        packet->items.clear();
        for (int i = 0; i < multiplayer_armor_size; i++) {
            ItemInstance item;
            item.setNull();
            safe_read(file, item, false);
            packet->items.push_back(item);
            inv_data.armor[i] = item;
        }
        rak_net_instance->sendTo(player->guid, *(Packet *) packet);
    }
    inventories[player->id] = inv_data;

    // Close File
    file.close();
}

// Save Player Data
template <typename T>
static void safe_write(std::ofstream &file, const T &obj) {
    file.write((char *) &obj, sizeof(obj));
}
static void save(ServerPlayer *player) {
    // Open File
    std::ofstream file(get_data_file(player), std::ios::binary);
    if (!file) {
        return;
    }

    // Get Inventory
    InventoryData &inv_data = inventories[player->id];
    const bool save_inv = !player->inventory->is_creative;
    if (!save_inv) {
        inv_data.inv.clear();
    }

    // Write Data
    const Pos spawn_pos = player->getRespawnPosition();
    const PlayerData data = {
        .x = player->x,
        .y = player->y,
        .z = player->z,
        .yaw = player->yaw,
        .pitch = player->pitch,
        .has_spawn = player->hasRespawnPosition(),
        .spawn_x = spawn_pos.x,
        .spawn_y = spawn_pos.y,
        .spawn_z = spawn_pos.z,
        .health = player->health,
        .fire_timer = player->fire_timer,
        .inventory_size = int(inv_data.inv.size())
    };
    safe_write(file, data);

    // Write Inventory
    if (save_inv) {
        std::vector<ItemInstance> items;
        items.insert(items.end(), inv_data.inv.begin(), inv_data.inv.end());
        items.insert(items.end(), inv_data.armor.begin(), inv_data.armor.end());
        for (const ItemInstance &item : items) {
            safe_write(file, item);
        }
    }

    // Close File
    file.close();
}
static void ServerSideNetworkHandler_onDisconnect_injection(ServerSideNetworkHandler_onDisconnect_t original, ServerSideNetworkHandler *self, const RakNet_RakNetGUID &guid) {
    // Save
    ServerPlayer *player = (ServerPlayer *) self->getPlayer(guid);
    if (player) {
        save(player);
        inventories.erase(player->id);
    }
    // Call Original Method
    original(self, guid);
}
static void Level_saveGame_injection(Level_saveGame_t original, Level *self) {
    // Save
    for (const Player *player : self->players) {
        save((ServerPlayer *) player);
    }
    // Call Original Method
    original(self);
}

// Init
void _init_server_playerdata() {
    // Receive Inventories
    overwrite_calls(ServerSideNetworkHandler_handle_SendInventoryPacket, ServerSideNetworkHandler_handle_SendInventoryPacket_injection);
    // Handle Saving
    overwrite_calls(ServerSideNetworkHandler_onDisconnect, ServerSideNetworkHandler_onDisconnect_injection);
    overwrite_calls(Level_saveGame, Level_saveGame_injection);
}