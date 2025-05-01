#include <symbols/minecraft.h>

#include <libreborn/patch.h>
#include <mods/feature/feature.h>

#include "internal.h"

// Properly Deallocate Pending Players
static void ServerSideNetworkHandler_onDisconnect_injection(ServerSideNetworkHandler_onDisconnect_t original, ServerSideNetworkHandler *self, const RakNet_RakNetGUID &guid) {
    // Call Original Method
    original(self, guid);
    // Free
    Player *player = self->popPendingPlayer(guid);
    if (player) {
        player->destructor_deleting();
    }
}

// More Reliable Block Placement On Servers
static bool TileItem_useOn_Level_setTileAndData_injection(Level *self, const int x, const int y, const int z, const int tile_id, const int data) {
    // Call Original Method
    const bool ret = self->setTileAndData(x, y, z, tile_id, data);
    // Send Packet
    if (ret) {
        UpdateBlockPacket *packet = UpdateBlockPacket::allocate();
        ((Packet *) packet)->constructor();
        packet->vtable = UpdateBlockPacket_vtable::base;
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

// Init
void _init_multiplayer_misc() {
    // Free Pending Players
    if (feature_has("Properly Free Pending Players", server_enabled)) {
        overwrite_calls(ServerSideNetworkHandler_onDisconnect, ServerSideNetworkHandler_onDisconnect_injection);
    }

    // Send Extra Packet For Block Placement On Servers
    if (feature_has("Reliable Block Placement On Servers", server_enabled)) {
        overwrite_call((void *) 0xcb784, Level_setTileAndData, TileItem_useOn_Level_setTileAndData_injection);
        overwrite_call((void *) 0x98c24, Level_setTile, TilePlanterItem_useOn_Level_setTile_injection);
        patch_vtable(ServerSideNetworkHandler_handle_UpdateBlockPacket, ServerSideNetworkHandler_handle_UpdateBlockPacket_injection);
    }
}