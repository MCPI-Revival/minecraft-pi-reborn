#include <functional>
#include <string>
#include <vector>
#include <queue>
#include <pthread.h>

#include <netdb.h>
#include <arpa/inet.h>

#include <symbols/minecraft.h>

#include <libreborn/patch.h>
#include <libreborn/env/servers.h>
#include <libreborn/env/env.h>

#include <mods/misc/misc.h>

#include "internal.h"

// Resolve DNS
static void resolve_address(std::string &address) {
    // Hints
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    // Resolve
    addrinfo *res = nullptr;
    const int status = getaddrinfo(address.c_str(), nullptr, &hints, &res);
    if (status != 0) {
        // Invalid Address
        DEBUG("Unable To Resolve: %s", address.c_str());
        address.clear();
        return;
    }
    // Convert To String
    char str[INET_ADDRSTRLEN];
    const void *addr = &((sockaddr_in *) res->ai_addr)->sin_addr;
    inet_ntop(AF_INET, addr, str, sizeof(str));
    freeaddrinfo(res);
    DEBUG("Resolved: %s: %s", address.c_str(), str);
    address = str;
}

// Does Resolution In A Separate Thread
static std::queue<ServerList::Entry> queue;
static pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;
static std::vector<ServerList::Entry> list;
static void *resolve_thread(MCPI_UNUSED void *data) {
    while (!queue.empty()) {
        ServerList::Entry entry = queue.front();
        queue.pop();
        resolve_address(entry.first);
        pthread_mutex_lock(&list_mutex);
        list.push_back(entry);
        pthread_mutex_unlock(&list_mutex);
    }
    return nullptr;
}
static void start_resolution() {
    // Load Server List
    ServerList server_list;
    const char *str = require_env(MCPI_SERVER_LIST_ENV);
    env_value_to_obj(server_list, str);
    for (const ServerList::Entry &entry : server_list.entries) {
        queue.push(entry);
    }
    // Start Thread
    pthread_t thread;
    pthread_create(&thread, nullptr, resolve_thread, nullptr);
    pthread_detach(thread);
}

// Iterate Server List
static void iterate_servers(const std::function<void(const char *address, ServerList::port_t port)> &callback) {
    // Loop
    pthread_mutex_lock(&list_mutex);
    for (const ServerList::Entry &entry : list) {
        if (!entry.first.empty() && entry.second > 0) {
            callback(entry.first.c_str(), entry.second);
        }
    }
    pthread_mutex_unlock(&list_mutex);
}

// Ping External Servers
static void RakNetInstance_pingForHosts_injection(RakNetInstance_pingForHosts_t original, RakNetInstance *rak_net_instance, const int32_t base_port) {
    // Call Original Method
    original(rak_net_instance, base_port);

    // Get RakNet::RakPeer
    RakNet_RakPeer *rak_peer = rak_net_instance->peer;

    // Add External Servers
    iterate_servers([rak_peer](const char *address, const ServerList::port_t port) {
        rak_peer->Ping(address, port, true, 0);
    });
}

// Init
void _init_multiplayer_server_list() {
    overwrite_calls(RakNetInstance_pingForHosts, RakNetInstance_pingForHosts_injection);
    misc_run_on_init([](MCPI_UNUSED Minecraft *mc) {
        start_resolution();
    });
}
