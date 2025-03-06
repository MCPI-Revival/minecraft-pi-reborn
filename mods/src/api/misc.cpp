#include <libreborn/patch.h>

#include <mods/feature/feature.h>

#include "internal.h"

// Fix HUD Spectating Other Players
template <typename... Args>
static void ItemInHandRenderer_render_injection(const std::function<void(ItemInHandRenderer *, Args...)> &original, ItemInHandRenderer *self, Args... args) {
    // "Fix" Current Player
    LocalPlayer *&player = self->minecraft->player;
    LocalPlayer *old_player = player;
    Mob *camera = self->minecraft->camera;
    if (camera && camera->isPlayer()) {
        player = (LocalPlayer *) camera;
    }
    // Call Original Method
    original(self, std::forward<Args>(args)...);
    // Revert "Fix"
    player = old_player;
}

// Fix Crash When Camera Entity Is Removed
static void LevelRenderer_entityRemoved_injection(LevelRenderer *self, Entity *entity) {
    // Call Original Method
    LevelListener_entityRemoved->get(false)((LevelListener *) self, entity);
    // Fix Camera
    Minecraft *minecraft = self->minecraft;
    if ((Entity *) minecraft->camera == entity) {
        minecraft->camera = (Mob *) minecraft->player;
    }
}

// Close Sockets
static void CommandServer__close_injection_1(CommandServer__close_t original, CommandServer *self) {
    // Close
    for (const ConnectedClient &client : self->clients) {
        close(client.sock);
    }
    self->clients.clear();
    // Call Original Method
    original(self);
}
static void Minecraft_leaveGame_injection(Minecraft_leaveGame_t original, Minecraft *self, const bool save_remote_level) {
    // Destroy Server
    CommandServer *&server = self->command_server;
    if (server) {
        server->destructor(0);
        ::operator delete(server);
        server = nullptr;
    }
    // Call Original Method
    original(self, save_remote_level);
}
static bool CommandServer__updateClient_injection_1(CommandServer__updateClient_t original, CommandServer *self, ConnectedClient &client) {
    // Call Original Method
    const bool ret = original(self, client);
    // Close Socket If Needed
    if (!ret) {
        close(client.sock);
    }
    return ret;
}

// Properly Teleport Players
void api_update_entity_position(const Entity *entity) {
    MovePlayerPacket *packet = MovePlayerPacket::allocate(); // Despite The Name, This Supports All Entities
    ((Packet *) packet)->constructor();
    packet->vtable = MovePlayerPacket_vtable::base;
    packet->x = entity->x;
    packet->y = entity->y - entity->height_offset;
    packet->z = entity->z;
    packet->yaw = entity->yaw;
    packet->pitch = entity->pitch;
    packet->entity_id = entity->id;
    entity->level->rak_net_instance->send(*(Packet *) packet);
    packet->destructor_deleting();
}
static void CommandServer_parse_Entity_moveTo_injection(Entity *self, const float x, const float y, const float z, const float yaw, const float pitch) {
    self->moveTo(x, y, z, yaw, pitch);
    api_update_entity_position(self);
}
static void ClientSideNetworkHandler_handle_MovePlayerPacket_injection(ClientSideNetworkHandler_handle_MovePlayerPacket_t original, ClientSideNetworkHandler *self, const RakNet_RakNetGUID &rak_net_guid, MovePlayerPacket *packet) {
    if (self->level) {
        Entity *entity = self->level->getEntity(packet->entity_id);
        if (entity) {
            if (entity == (Entity *) self->minecraft->player) {
                // Just Teleport
                entity->moveTo(packet->x, packet->y, packet->z, packet->yaw, packet->pitch);
            } else {
                // Call Original Method
                original(self, rak_net_guid, packet);
            }
        }
    }
}

// Init
void _init_api_misc() {
    // Bug Fixes
    if (feature_has("Fix HUD When Spectating Other Players", server_enabled)) {
        overwrite_calls(ItemInHandRenderer_render, ItemInHandRenderer_render_injection<float>);
        overwrite_calls(ItemInHandRenderer_renderScreenEffect, ItemInHandRenderer_render_injection<float>);
        overwrite_calls(ItemInHandRenderer_tick, ItemInHandRenderer_render_injection<>);
    }
    if (feature_has("Fix Crash When Spectated Entity Is Removed", server_enabled)) {
        patch_vtable(LevelRenderer_entityRemoved, LevelRenderer_entityRemoved_injection);
    }
    if (feature_has("Correctly Close API Sockets", server_enabled)) {
        overwrite_calls(CommandServer__close, CommandServer__close_injection_1);
        overwrite_calls(Minecraft_leaveGame, Minecraft_leaveGame_injection);
        overwrite_calls(CommandServer__updateClient, CommandServer__updateClient_injection_1);
    }
    if (feature_has("Fix Moving Players With The API In Multiplayer", server_enabled)) {
        overwrite_calls(ClientSideNetworkHandler_handle_MovePlayerPacket, ClientSideNetworkHandler_handle_MovePlayerPacket_injection);
        overwrite_call((void *) 0x6b6e8, Entity_moveTo, CommandServer_parse_Entity_moveTo_injection);
    }
}