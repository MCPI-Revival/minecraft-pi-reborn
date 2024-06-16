#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include <symbols/minecraft.h>
#include <libreborn/libreborn.h>

#include <mods/home/home.h>
#include <mods/init/init.h>
#include <mods/feature/feature.h>

// Load Server List
struct server_list_entry {
    std::string address;
    int port;
};
static std::vector<server_list_entry> server_list_entries;
static bool server_list_loaded = false;
static void load_servers() {
    // Prepare
    server_list_entries.clear();

    // Open Servers File
    std::string file(home_get());
    file.append("/servers.txt");

    // Create Stream
    std::ifstream server_list_file(file);

    if (!server_list_file.good()) {
        // Write Defaults
        std::ofstream server_list_file_output(file);
        server_list_file_output << "# External Servers File\n";
        server_list_file_output << "# Example: thebrokenrail.com\n";
        server_list_file_output.close();
        // Re-Open Stream
        server_list_file = std::ifstream(file);
    }

    // Check Servers File
    if (!server_list_file.is_open()) {
        ERR("Unable To Open %s", file.c_str());
    }

    // Iterate Lines
    {
        std::string line;
        while (std::getline(server_list_file, line)) {
            // Check Line
            if (!line.empty()) {
                if (line[0] == '#') {
                    continue;
                }
                // Parse
                std::string address;
                std::string port_str;
                size_t separator_pos = line.find_last_of(':');
                if (separator_pos == std::string::npos) {
                    port_str = "19132";
                    address = line;
                } else {
                    address = line.substr(0, separator_pos);
                    port_str = line.substr(separator_pos + 1);
                }
                // Check Line
                if (address.empty() || port_str.empty() || port_str.find_first_not_of("0123456789") != std::string::npos) {
                    // Invalid Line
                    WARN("Invalid Server: %s", line.c_str());
                    continue;
                }
                // Parse Port
                int port = std::stoi(port_str);
                // Done
                server_list_entry entry = {
                    .address = address,
                    .port = port
                };
                server_list_entries.push_back(entry);
            }
        }
    }

    // Close
    server_list_file.close();
}

// Iterare Server List
static void iterate_servers(const std::function<void(const char *address, int port)> &callback) {
    // Load
    if (!server_list_loaded) {
        load_servers();
        server_list_loaded = true;
    }

    // Loop
    for (const server_list_entry &entry : server_list_entries) {
        callback(entry.address.c_str(), entry.port);
    }
}

// Ping External Servers
static void RakNetInstance_pingForHosts_injection(RakNetInstance_pingForHosts_t original, RakNetInstance *rak_net_instance, int32_t base_port) {
    // Call Original Method
    original(rak_net_instance, base_port);

    // Get RakNet::RakPeer
    RakNet_RakPeer *rak_peer = rak_net_instance->peer;

    // Add External Servers
    iterate_servers([rak_peer](const char *address, int port) {
        rak_peer->Ping(address, port, true, 0);
    });
}

// Init
void init_multiplayer() {
    // Inject Code
    if (feature_has("External Server Support", server_disabled)) {
        overwrite_calls(RakNetInstance_pingForHosts, RakNetInstance_pingForHosts_injection);
    }
}
