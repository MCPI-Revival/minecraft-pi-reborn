#pragma once

#include <string>
#include <istream>
#include <map>

class ServerProperties {
    std::map<std::string, std::string> properties;

public:
    void load(std::istream &stream);

    [[nodiscard]] std::string get_string(const std::string &name, const std::string &def) const;
    [[nodiscard]] int get_int(const std::string &name, const std::string &def) const;
    [[nodiscard]] bool get_bool(const std::string &name, const std::string &def) const;
};