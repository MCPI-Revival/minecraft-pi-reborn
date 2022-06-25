#pragma once

#include <string>
#include <istream>
#include <map>

class ServerProperties {

private:
    std::map<std::string, std::string> properties;

public:
    void load(std::istream& fstream);

    std::string get_string(std::string const& name, std::string const& def);
    int get_int(std::string const& name, std::string const& def);
    bool get_bool(std::string const& name, std::string const& def);
};