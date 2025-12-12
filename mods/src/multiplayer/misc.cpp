#include <symbols/ServerSideNetworkHandler.h>
#include <symbols/Player.h>
#include <symbols/Level.h>
#include <symbols/UpdateBlockPacket.h>
#include <symbols/Packet.h>
#include <symbols/RakNetInstance.h>
#include <symbols/ClientSideNetworkHandler.h>
#include <symbols/Tile.h>
#include <symbols/Minecraft.h>
#include <symbols/LocalPlayer.h>
#include <symbols/RemovePlayerPacket.h>

#include <libreborn/patch.h>
#include <mods/feature/feature.h>

#include "internal.h"

// Properly Deallocate Pending Players
static void free_pending_players(ServerSideNetworkHandler *self, const RakNet_RakNetGUID &guid) {
    while (true) {
        Player *player = self->popPendingPlayer(guid);
        if (!player) {
            break;
        }
        player->destructor_deleting();
    }
}
static void ServerSideNetworkHandler_onDisconnect_injection(ServerSideNetworkHandler_onDisconnect_t original, ServerSideNetworkHandler *self, const RakNet_RakNetGUID &guid) {
    // Call Original Method
    original(self, guid);
    // Free
    free_pending_players(self, guid);
}
static void ServerSideNetworkHandler_handle_LoginPacket_injection(ServerSideNetworkHandler_handle_LoginPacket_t original, ServerSideNetworkHandler *self, const RakNet_RakNetGUID &guid, LoginPacket *packet) {
    // Free
    free_pending_players(self, guid);
    // Call Original Method
    original(self, guid, packet);
}

// More Reliable Block Placement On Servers
static bool TileItem_useOn_Level_setTileAndData_injection(Level *self, const int x, const int y, const int z, const int tile_id, const int data) {
    // Call Original Method
    const bool ret = self->setTileAndData(x, y, z, tile_id, data);
    // Send Packet
    if (ret) {
        UpdateBlockPacket *packet = UpdateBlockPacket::allocate();
        ((Packet *) packet)->constructor();
        packet->vtable = UpdateBlockPacket::VTable::base;
        packet->x = x;
        packet->y = y;
        packet->z = z;
        packet->tile_id = tile_id;
        packet->data = data;
        self->rak_net_instance->send(*(Packet *) packet);
        packet->destructor_deleting();
    }
    // Return
    return ret;
}
static bool TilePlanterItem_useOn_Level_setTile_injection(Level *self, const int x, const int y, const int z, const int tile_id) {
    return TileItem_useOn_Level_setTileAndData_injection(self, x, y, z, tile_id, 0);
}
static void ServerSideNetworkHandler_handle_UpdateBlockPacket_injection(ServerSideNetworkHandler *self, const RakNet_RakNetGUID &guid, UpdateBlockPacket *packet) {
    Level *level = self->level;
    if (!level) {
        return;
    }
    Player *player = self->getPlayer(guid);
    if (!player) {
        return;
    }
    // Place
    const int x = packet->x;
    const int y = packet->y;
    const int z = packet->z;
    const int tile_id = Tile::transformToValidBlockId(packet->tile_id, x, y, z);
    level->setTileAndData(x, y, z, tile_id, packet->data);
    Tile::tiles[tile_id]->setPlacedBy(level, x, y, z, (Mob *) player);
}

// Prevent Removing Local Player
static void ClientSideNetworkHandler_handle_RemovePlayerPacket_injection(ClientSideNetworkHandler_handle_RemovePlayerPacket_t original, ClientSideNetworkHandler *self, const RakNet_RakNetGUID &guid, RemovePlayerPacket *packet) {
    // Check
    const Minecraft *minecraft = self->minecraft;
    if (minecraft) {
        const LocalPlayer *player = minecraft->player;
        if (player && player->id == packet->entity_id) {
            return;
        }
    }
    // Call Original Method
    original(self, guid, packet);
}

// Init
void _init_multiplayer_misc() {
    // Free Pending Players
    if (feature_has("Properly Free Pending Players", server_enabled)) {
        overwrite_calls(ServerSideNetworkHandler_onDisconnect, ServerSideNetworkHandler_onDisconnect_injection);
        overwrite_calls(ServerSideNetworkHandler_handle_LoginPacket, ServerSideNetworkHandler_handle_LoginPacket_injection);
    }

    // Send Extra Packet For Block Placement On Servers
    if (feature_has("Reliable Block Placement On Servers", server_enabled)) {
        overwrite_call((void *) 0xcb784, Level_setTileAndData, TileItem_useOn_Level_setTileAndData_injection);
        overwrite_call((void *) 0x98c24, Level_setTile, TilePlanterItem_useOn_Level_setTile_injection);
        patch_vtable(ServerSideNetworkHandler_handle_UpdateBlockPacket, ServerSideNetworkHandler_handle_UpdateBlockPacket_injection);
    }

    // Prevent Removing Local Player
    if (feature_has("Prevent Removing Local Player", server_disabled)) {
        overwrite_calls(ClientSideNetworkHandler_handle_RemovePlayerPacket, ClientSideNetworkHandler_handle_RemovePlayerPacket_injection);
    }
}