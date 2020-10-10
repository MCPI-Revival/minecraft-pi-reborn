#include "server_properties.h"

static bool is_true(std::string const& val) {
    return (val == "true" || val == "yes" || val == "1");
}

void ServerProperties::load(std::istream& stream) {
    std::string line;
    while (std::getline(stream, line)) {
        if (line.length() > 0) {
            if (line[0] == '#') {
                continue;
            }
            size_t i = line.find('=');
            if (i == std::string::npos) {
                continue;
            }
            properties.insert(std::pair<std::string, std::string>(line.substr(0, i), line.substr(i + 1)));
        }
    }
}

std::string ServerProperties::get_string(std::string const& name, std::string const& def) {
    return properties.count(name) > 0 ? properties.at(name) : def;
}

int ServerProperties::get_int(std::string const& name, std::string const& def) {
    return properties.count(name) > 0 ? std::stoi(properties.at(name)) : std::stoi(def);
}

bool ServerProperties::get_bool(std::string const& name, std::string const& def) {
    if (properties.count(name) > 0) {
        std::string const& val = properties.at(name);
        return is_true(val);
    }
    return is_true(def);
}
