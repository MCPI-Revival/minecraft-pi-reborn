#pragma once

#include <string>
#include <unordered_set>

#include <symbols/minecraft.h>

__attribute__((visibility("internal"))) std::string get_rak_net_guid_ip(RakNet_RakPeer *rak_peer, const RakNet_RakNetGUID &guid);

__attribute__((visibility("internal"))) void handle_commands(Minecraft *minecraft);
__attribute__((visibility("internal"))) void start_reading_commands();
__attribute__((visibility("internal"))) void stop_reading_commands();

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
__attribute__((visibility("internal"))) extern Blacklist blacklist;