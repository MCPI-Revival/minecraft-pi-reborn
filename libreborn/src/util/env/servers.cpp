#include <fstream>
#include <sstream>
#include <limits>

#include <libreborn/env/servers.h>
#include <libreborn/env/flags.h>
#include <libreborn/util/util.h>

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
void ServerList::load(const std::string &str) {
    // Clear
    entries.clear();

    // Open File
    std::stringstream server_list_file(str);

    // Iterate Lines
    std::string line;
    while (std::getline(server_list_file, line, FLAG_SEPERATOR_CHAR)) {
        // Check Line
        if (!line.empty()) {
            // Parse
            std::string address;
            std::string port_str;
            const size_t separator_pos = line.find_last_of(PORT_SEPERATOR_CHAR);
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
}

// Save
std::string ServerList::to_string() const {
    std::stringstream out;
    for (const Entry &entry : entries) {
        out << entry.first << PORT_SEPERATOR_CHAR << std::to_string(entry.second) << FLAG_SEPERATOR_CHAR;
    }
    return out.str();
}
