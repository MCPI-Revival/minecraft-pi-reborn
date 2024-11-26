#include <fstream>

#include <mods/server/server_properties.h>

std::vector<const ServerProperty *> &ServerProperty::get_all() {
    static std::vector<const ServerProperty *> out;
    return out;
}

static bool is_true(std::string const& val) {
    return val == "true" || val == "yes" || val == "1";
}

void ServerProperties::load(std::istream &stream) {
    std::string line;
    while (std::getline(stream, line)) {
        if (line.length() > 0) {
            if (line[0] == '#') {
                continue;
            }
            const size_t i = line.find('=');
            if (i == std::string::npos) {
                continue;
            }
            properties.insert(std::pair(line.substr(0, i), line.substr(i + 1)));
        }
    }
}

std::string ServerProperties::get_string(const ServerProperty &property) const {
    return properties.contains(property.key) ? properties.at(property.key) : property.def;
}

int ServerProperties::get_int(const ServerProperty &property) const {
    return std::stoi(get_string(property));
}

bool ServerProperties::get_bool(const ServerProperty &property) const {
    return is_true(get_string(property));
}
