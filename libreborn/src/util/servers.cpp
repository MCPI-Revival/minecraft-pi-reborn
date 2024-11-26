#include <fstream>
#include <sstream>
#include <limits>

#include <libreborn/servers.h>
#include <libreborn/util.h>

// File
static std::string get_server_list_path() {
    return home_get() + "/servers.txt";
}

// Seperator
#define PORT_SEPERATOR_CHAR ':'

// Load
ServerList::port_t ServerList::parse_port(const std::string &s) {
    unsigned long port = std::strtoul(s.c_str(), nullptr, 10);
    constexpr port_t max = std::numeric_limits<port_t>::max();
    if (port > max) {
        port = max;
    }
    return static_cast<port_t>(port);
}
ServerList::ServerList() {
    // Open File
    std::ifstream server_list_file(get_server_list_path());
    if (!server_list_file) {
        // File Does Not Exist
        return;
    }

    // Iterate Lines
    std::string line;
    while (std::getline(server_list_file, line)) {
        // Check Line
        if (!line.empty()) {
            // Skip Comments
            if (line[0] == '#') {
                continue;
            }
            // Parse
            std::string address;
            std::string port_str;
            size_t separator_pos = line.find_last_of(PORT_SEPERATOR_CHAR);
            if (separator_pos == std::string::npos) {
                port_str = std::to_string(DEFAULT_MULTIPLAYER_PORT);
                address = line;
            } else {
                address = line.substr(0, separator_pos);
                port_str = line.substr(separator_pos + 1);
            }
            // Done
            entries.emplace_back(address, parse_port(port_str));
        }
    }

    // Close
    server_list_file.close();
}

// Save
std::string ServerList::to_string() const {
    std::stringstream out;
    out << "# External Servers File\n";
    for (const Entry &entry : entries) {
        out << entry.first << PORT_SEPERATOR_CHAR << std::to_string(entry.second) << '\n';
    }
    return out.str();
}
void ServerList::save() const {
    // Open File
    std::ofstream file(get_server_list_path());
    if (!file) {
        // Failure
        WARN("Unable To Save Server List");
    }
    // Write
    file << to_string();
    // Close
    file.close();
}
