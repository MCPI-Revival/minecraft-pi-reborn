#pragma once

#include <string>
#include <map>
#include <vector>

struct ServerProperty {
    static std::vector<const ServerProperty *> &get_all();
    const std::string key;
    const std::string def;
    const std::string comment;
    ServerProperty(const std::string &key_, const std::string &def_, const std::string &comment_):
        key(key_),
        def(def_),
        comment(comment_) {
        get_all().push_back(this);
    }
};

class ServerProperties {
    std::map<std::string, std::string> properties;

public:
    void load(std::istream &stream);

    [[nodiscard]] std::string get_string(const ServerProperty &property) const;
    [[nodiscard]] int get_int(const ServerProperty &property) const;
    [[nodiscard]] bool get_bool(const ServerProperty &property) const;
};