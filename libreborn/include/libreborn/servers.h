#pragma once

#include <utility>
#include <string>
#include <vector>

// Parse servers.txt
struct ServerList {
    // Type
    typedef unsigned short port_t;
    typedef std::pair<std::string, port_t> Entry;
    // Load
    static port_t parse_port(const std::string &s);
    ServerList();
    // Save
    std::string to_string() const;
    void save() const;
    // Entries
    std::vector<Entry> entries{};
};