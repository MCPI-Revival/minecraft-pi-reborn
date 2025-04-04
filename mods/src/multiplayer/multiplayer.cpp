#include <functional>
#include <string>
#include <vector>

#include <symbols/minecraft.h>

#include <libreborn/patch.h>
#include <libreborn/env/servers.h>
#include <libreborn/env/env.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>

// Iterate Server List
static void iterate_servers(const std::function<void(const char *address, ServerList::port_t port)> &callback) {
    // Load
    static ServerList server_list;
    static bool loaded = false;
    if (!loaded) {
        const char *str = require_env(MCPI_SERVER_LIST_ENV);
        env_value_to_obj(server_list, str);
        loaded = true;
    }
    // Loop
    for (const ServerList::Entry &entry : server_list.entries) {
        if (!entry.first.empty() && entry.second > 0) {
            callback(entry.first.c_str(), entry.second);
        }
    }
}

// Ping External Servers
static void RakNetInstance_pingForHosts_injection(RakNetInstance_pingForHosts_t original, RakNetInstance *rak_net_instance, const int32_t base_port) {
    // Call Original Method
    original(rak_net_instance, base_port);

    // Get RakNet::RakPeer
    RakNet_RakPeer *rak_peer = rak_net_instance->peer;

    // Add External Servers
    iterate_servers([rak_peer](const char *address, ServerList::port_t port) {
        rak_peer->Ping(address, port, true, 0);
    });
}

// Fix Bug Where RakNetInstance Starts Pinging Potential Servers Before The "Join Game" Screen Is Opened
static RakNetInstance *RakNetInstance_injection(RakNetInstance_constructor_t original, RakNetInstance *rak_net_instance) {
    // Call Original Method
    RakNetInstance *result = original(rak_net_instance);
    // Fix
    rak_net_instance->pinging_for_hosts = false;
    // Return
    return result;
}

// Init
void init_multiplayer() {
    // Inject Code
    if (feature_has("External Server Support", server_disabled)) {
        overwrite_calls(RakNetInstance_pingForHosts, RakNetInstance_pingForHosts_injection);
    }

    // Fix Bug Where RakNetInstance Starts Pinging Potential Servers Before The "Join Game" Screen Is Opened
    if (feature_has("Prevent Unnecessary Server Pinging", server_enabled)) {
        overwrite_calls(RakNetInstance_constructor, RakNetInstance_injection);
    }
}
