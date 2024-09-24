#pragma once

#include <string>
#include <istream>
#include <map>
#include <vector>

struct ServerProperty {
    static std::vector<const ServerProperty *> &get_all();
    const char *const key;
    const char *const def;
    const char *const comment;
    ServerProperty(const char *const key_, const char *const def_, const char *const comment_):
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