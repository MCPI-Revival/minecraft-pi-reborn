#include <libreborn/util/util.h>
#include <libreborn/log.h>

#include <mods/server/server.h>

#include <fstream>
#include <unordered_set>

#include "internal.h"

// Get Path Of Blacklist (Or Whitelist) File
std::string Blacklist::get_name(const bool upper) const {
    std::string ret = is_white ? "white" : "black";
    if (upper) {
        ret[0] = std::toupper(ret[0]);
    }
    ret += "list";
    return ret;
}
static std::string get_blacklist_file() {
    std::string file = home_get();
    file += '/';
    file += blacklist.get_name(false);
    file += ".txt";
    return file;
}

// Load/Save
void Blacklist::load() {
    // Load
    ips.clear();
    std::ifstream blacklist_file(get_blacklist_file(), std::ios::binary);
    if (blacklist_file) {
        // Read File
        std::string line;
        while (std::getline(blacklist_file, line)) {
            // Check Line
            if (!line.empty() && line[0] != '#') {
                ips.insert(line);
            }
        }
        blacklist_file.close();
    }
    // Save Loaded Data
    save();
}
void Blacklist::save() const {
    std::ofstream blacklist_file(get_blacklist_file(), std::ios::binary);
    if (blacklist_file) {
        blacklist_file << "# " << get_name(true) << "; Each Line Is One IP Address" << std::endl;
        for (const std::string &ip : ips) {
            blacklist_file << ip << std::endl;
        }
        blacklist_file.close();
        if (!blacklist_file) {
            ERR("Unable To Save %s", get_name(true).c_str());
        }
    } else {
        ERR("Unable To Open %s For Saving", get_name(true).c_str());
    }
}

// Miscellaneous
bool Blacklist::contains(const std::string &ip) const {
    return ips.contains(ip);
}

// Get IP Address
std::string get_rak_net_guid_ip(RakNet_RakPeer *rak_peer, const RakNet_RakNetGUID &guid) {
    RakNet_SystemAddress address = rak_peer->GetSystemAddressFromGuid(guid);
    // Get IP
    return address.ToString(false, '|');
}

// Add/Remove
void Blacklist::add_ip(const std::string &ip) {
    ips.insert(ip);
    save();
    INFO("Added To %s: %s", get_name(true).c_str(), ip.c_str());
}
void Blacklist::remove_ip(const std::string &ip) {
    const bool ret = ips.contains(ip);
    if (ret) {
        ips.erase(ip);
        save();
        INFO("Removed From %s: %s", get_name(true).c_str(), ip.c_str());
    } else {
        WARN("%s Does Not Contain IP: %s", get_name(true).c_str(), ip.c_str());
    }
}

// Ban
void Blacklist::ban(const ServerPlayer *player) {
    // Kick Player
    server_kick(player);
    // Ban
    const std::string ip = get_rak_net_guid_ip(player->minecraft->rak_net_instance->peer, player->guid);
    if (is_white) {
        // Remove From Whitelist
        remove_ip(ip);
    } else {
        // Add To Blacklist
        add_ip(ip);
    }
}

// Global Instance
Blacklist blacklist;