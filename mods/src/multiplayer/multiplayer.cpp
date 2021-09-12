#ifdef MCPI_SERVER_MODE
#error "External Server Code Requires Client Mode"
#endif

#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "../home/home.h"
#include "../init/init.h"
#include "../feature/feature.h"

// Load Server List
struct server_list_entry {
    std::string address;
    int port;
};
static std::vector<struct server_list_entry> server_list_entries;
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
        server_list_file_output << "thebrokenrail.com\n";
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
            if (line.length() > 0) {
                if (line[0] == '#') {
                    continue;
                }
                // Parse
                std::string address;
                std::string port_str;
                // Add Default Port If Needed
                size_t last_colon = line.find_last_of(':');
                if (last_colon == std::string::npos) {
                    line.append(":19132");
                    last_colon = line.find_last_of(':');
                }
                // Loop
                for (std::string::size_type i = 0; i < line.length(); i++) {
                    if (i > last_colon) {
                        port_str.push_back(line[i]);
                    } else if (i < last_colon) {
                        address.push_back(line[i]);
                    }
                }
                // Check Line
                if (address.length() < 1 || port_str.length() < 1 || port_str.find_first_not_of("0123456789") != std::string::npos) {
                    // Invalid Line
                    WARN("Invalid Server: %s", line.c_str());
                    continue;
                }
                // Parse Port
                int port = std::stoi(port_str);

                // Done
                struct server_list_entry entry;
                entry.address = address;
                entry.port = port;
                server_list_entries.push_back(entry);
            }
        }
    }

    // Close
    server_list_file.close();
}

// Iterare Server List
static void iterate_servers(std::function<void(const char *address, int port)> callback) {
    // Load
    if (!server_list_loaded) {
        load_servers();
        server_list_loaded = true;
    }

    // Loop
    for (std::vector<struct server_list_entry>::size_type i = 0; i < server_list_entries.size(); i++) {
        struct server_list_entry entry = server_list_entries[i];
        callback(entry.address.c_str(), entry.port);
    }
}

// Ping External Servers
static void RakNetInstance_pingForHosts_injection(unsigned char *rak_net_instance, int32_t base_port) {
    // Call Original Method
    (*RakNetInstance_pingForHosts)(rak_net_instance, base_port);

    // Get RakNet::RakPeer
    unsigned char *rak_peer = *(unsigned char **) (rak_net_instance + RakNetInstance_peer_property_offset);

    // Add External Servers
    iterate_servers([rak_peer](const char *address, int port) {
        (*RakNet_RakPeer_Ping)(rak_peer, address, port, 1, 0);
    });
}

// Init
void init_multiplayer() {
    // Inject Code
    if (feature_has("External Server Support", 0)) {
        patch_address(RakNetInstance_pingForHosts_vtable_addr, (void *) RakNetInstance_pingForHosts_injection);
    }
}
