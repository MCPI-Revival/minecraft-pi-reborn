#pragma once

__attribute__((visibility("internal"))) bool is_whitelist();
__attribute__((visibility("internal"))) std::string get_blacklist_file();
__attribute__((visibility("internal"))) bool is_ip_in_blacklist(const char *ip);

__attribute__((visibility("internal"))) char *get_rak_net_guid_ip(RakNet_RakPeer *rak_peer, const RakNet_RakNetGUID &guid);

__attribute__((visibility("internal"))) void handle_commands(Minecraft *minecraft);
__attribute__((visibility("internal"))) void start_reading_commands();
__attribute__((visibility("internal"))) void stop_reading_commands();