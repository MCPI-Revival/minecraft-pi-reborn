#pragma once

#include <string>
#include <unordered_set>

#include <symbols/RakNet_RakPeer.h>
#include <symbols/ServerPlayer.h>
#include <symbols/Level.h>
#include <symbols/RakNetInstance.h>
#include <symbols/Minecraft.h>
#include <symbols/RakNet_AddressOrGUID.h>
#include <symbols/NetEventCallback.h>
#include <symbols/ServerSideNetworkHandler.h>
#include <symbols/SendInventoryPacket.h>
#include <symbols/ExternalFileLevelStorage.h>
#include <symbols/Inventory.h>
#include <symbols/ContainerSetContentPacket.h>
#include <symbols/Packet.h>
#include <symbols/LevelSettings.h>
#include <symbols/ProgressScreen.h>
#include <symbols/ChunkSource.h>
#include <symbols/CompoundTag.h>

MCPI_INTERNAL std::string get_rak_net_guid_ip(const RakNet_RakPeer *rak_peer, const RakNet_RakNetGUID &guid);

MCPI_INTERNAL void handle_commands(Minecraft *minecraft);

MCPI_INTERNAL void _init_server_playerdata();
MCPI_INTERNAL void _load_playerdata(ServerPlayer *player);

// Blacklist/Whitelist
struct Blacklist {
    bool is_white;
    // General
    void load();
    void save() const;
    // Miscellaneous
    [[nodiscard]] std::string get_name(bool upper) const;
    [[nodiscard]] bool contains(const std::string &ip) const;
    // Add/Remove
    void add_ip(const std::string &ip);
    void remove_ip(const std::string &ip);
    // Ban
    void ban(const ServerPlayer *player);
private:
    // Data
    std::unordered_set<std::string> ips;
};
MCPI_INTERNAL extern Blacklist blacklist;