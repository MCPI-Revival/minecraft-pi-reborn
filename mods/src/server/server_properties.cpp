#include <mods/server/server_properties.h>

static bool is_true(std::string const& val) {
    return (val == "true" || val == "yes" || val == "1");
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

std::string ServerProperties::get_string(const std::string &name, const std::string &def) const {
    return properties.contains(name) ? properties.at(name) : def;
}

int ServerProperties::get_int(const std::string &name, const std::string &def) const {
    return properties.contains(name) ? std::stoi(properties.at(name)) : std::stoi(def);
}

bool ServerProperties::get_bool(const std::string &name, const std::string &def) const {
    if (properties.contains(name)) {
        const std::string &val = properties.at(name);
        return is_true(val);
    }
    return is_true(def);
}
