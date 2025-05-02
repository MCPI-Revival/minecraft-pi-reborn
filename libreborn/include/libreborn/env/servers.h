#pragma once

#include <utility>
#include <string>
#include <vector>

// Parse servers.txt
struct ServerList {
    ServerList();
    // Type
    typedef unsigned short port_t;
    typedef std::pair<std::string, port_t> Entry;
    // Load
    static port_t parse_port(const std::string &s);
    void load(const std::string &str);
    // Save
    [[nodiscard]] std::string to_string() const;
    // Entries
    std::vector<Entry> entries;
};