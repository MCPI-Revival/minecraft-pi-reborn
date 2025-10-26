#pragma once

#include <string>
#include <unordered_set>

#include <symbols/minecraft.h>

MCPI_INTERNAL std::string get_rak_net_guid_ip(RakNet_RakPeer *rak_peer, const RakNet_RakNetGUID &guid);

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