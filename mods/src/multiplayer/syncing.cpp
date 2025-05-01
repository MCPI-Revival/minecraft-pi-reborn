#include <symbols/minecraft.h>
#include <libreborn/patch.h>

#include <mods/feature/feature.h>

#include "internal.h"

// Fix Fire Syncing
#define FLAG_ONFIRE 0
static void Mob_baseTick_injection_fire_syncing(Mob_baseTick_t original, Mob *self) {
    // Fix Fire Timer
    if (self->level->is_client_side) {
        self->fire_timer = 0;
    }
    // Call Original Method
    original(self);
    // Sync Data
    if (!self->level->is_client_side) {
        self->setSharedFlag(FLAG_ONFIRE, self->fire_timer > 0);
    }
}
static bool Entity_isOnFire_injection(Entity_isOnFire_t original, Entity *self) {
    // Call Original Method
    bool ret = original(self);

    // Check Shared Data
    bool shared_data = false;
    if (self->isMob()) {
        shared_data = ((Mob *) self)->getSharedFlag(FLAG_ONFIRE);
    }
    if (shared_data) {
        ret = true;
    }

    // Return
    return ret;
}

// Fix Sneaking Syncing
#define FLAG_SNEAKING 1
#define PLAYER_ACTION_STOP_SNEAKING 100
#define PLAYER_ACTION_START_SNEAKING 101
static void LocalPlayer_tick_injection(LocalPlayer_tick_t original, LocalPlayer *self) {
    // Call Original Method
    original(self);
    // Sync Data
    if (!self->level->is_client_side) {
        self->setSharedFlag(FLAG_SNEAKING, self->isSneaking());
    } else {
        const bool real = self->isSneaking();
        const bool synced = self->getSharedFlag(FLAG_SNEAKING);
        if (real != synced) {
            // Send Packet To Server
            PlayerActionPacket *packet = PlayerActionPacket::allocate();
            ((Packet *) packet)->constructor();
            packet->vtable = PlayerActionPacket_vtable::base;
            packet->entity_id = self->id;
            packet->action = real ? PLAYER_ACTION_START_SNEAKING : PLAYER_ACTION_STOP_SNEAKING;
            self->minecraft->rak_net_instance->send(*(Packet *) packet);
            packet->destructor_deleting();
        }
    }
}
static void ServerSideNetworkHandler_handle_PlayerActionPacket_injection(ServerSideNetworkHandler_handle_PlayerActionPacket_t original, ServerSideNetworkHandler *self, const RakNet_RakNetGUID &rak_net_guid, PlayerActionPacket *packet) {
    // Call Original Method
    original(self, rak_net_guid, packet);

    // Handle Sneaking
    const bool is_sneaking = packet->action == PLAYER_ACTION_START_SNEAKING;
    if (self->level != nullptr && (is_sneaking || packet->action == PLAYER_ACTION_STOP_SNEAKING)) {
        Entity *entity = self->level->getEntity(packet->entity_id);
        if (entity != nullptr && entity->isPlayer()) {
            ((Player *) entity)->setSharedFlag(FLAG_SNEAKING, is_sneaking);
        }
    }
}

// Fix Tile Entity Syncing
template <typename... Args>
static bool LevelChunk_setTile_injection(const std::function<bool(LevelChunk *, int, int, int, int, Args...)> &original, LevelChunk *self, const int x, const int y, const int z, const int tile_id, Args... args) {
    // Call Original Method
    const bool ret = original(self, x, y, z, tile_id, std::forward<Args>(args)...);
    // Create Tile Entity If Needed
    if (ret && self->level->is_client_side && tile_id != 0 && Tile::isEntityTile[tile_id]) {
        self->getTileEntity(x, y, z);
    }
    // Return
    return ret;
}

// Fix Spawn Position Syncing
static void ServerSideNetworkHandler_onReady_ClientGeneration_injection(ServerSideNetworkHandler_onReady_ClientGeneration_t original, ServerSideNetworkHandler *self, const RakNet_RakNetGUID &rak_net_guid) {
    // Call Original Method
    original(self, rak_net_guid);
    // Get Spawn Position
    Pos pos = self->level->getSharedSpawnPos();
    Player *player = self->getPlayer(rak_net_guid);
    if (player && player->hasRespawnPosition()) {
        pos = player->getRespawnPosition();
    }
    // Send Spawn Position
    SetSpawnPositionPacket *packet = SetSpawnPositionPacket::allocate();
    ((Packet *) packet)->constructor();
    packet->vtable = SetSpawnPositionPacket_vtable::base;
    packet->x = pos.x;
    packet->y = pos.y;
    packet->z = pos.z;
    packet->priority = HIGH_PRIORITY;
    packet->reliability = RELIABLE_ORDERED;
    self->rak_net_instance->sendTo(rak_net_guid, *(Packet *) packet);
    packet->destructor_deleting();
}

// Init
void _init_multiplayer_syncing() {
    // Fix Fire Syncing
    if (feature_has("Fix Fire Syncing", server_enabled)) {
        overwrite_calls(Mob_baseTick, Mob_baseTick_injection_fire_syncing);
        overwrite_calls(Entity_isOnFire, Entity_isOnFire_injection);
    }

    // Fix Sneaking Syncing
    if (feature_has("Fix Sneaking Syncing", server_enabled)) {
        overwrite_calls(LocalPlayer_tick, LocalPlayer_tick_injection);
        overwrite_calls(ServerSideNetworkHandler_handle_PlayerActionPacket, ServerSideNetworkHandler_handle_PlayerActionPacket_injection);
    }

    // Fix Tile Entity Syncing
    if (feature_has("Fix Tile Entity Syncing", server_enabled)) {
        overwrite_calls(LevelChunk_setTile, LevelChunk_setTile_injection<>);
        overwrite_calls(LevelChunk_setTileAndData, LevelChunk_setTile_injection<int>);
    }

    // Fix Spawn Position Syncing
    if (feature_has("Fix Spawn Position Syncing", server_enabled)) {
        overwrite_calls(ServerSideNetworkHandler_onReady_ClientGeneration, ServerSideNetworkHandler_onReady_ClientGeneration_injection);
    }
}