#include <fstream>

#include <libreborn/util/string.h>

#include <mods/server/properties.h>

// Load File
static constexpr char comment_char = '#';
static constexpr char key_value_seperator = '=';
void ServerProperties::load(std::istream &stream) {
    std::string line;
    while (std::getline(stream, line)) {
        trim(line);
        if (!line.empty()) {
            if (line.front() == comment_char) {
                continue;
            }
            const size_t i = line.find(key_value_seperator);
            if (i == std::string::npos) {
                continue;
            }
            properties.insert({line.substr(0, i), line.substr(i + 1)});
        }
    }
}
std::optional<std::string> ServerProperties::get(const std::string &key) const {
    return properties.contains(key) ? std::optional(properties.at(key)) : std::nullopt;
}

// Get All Possible Properties
std::vector<const ServerPropertyBase *> &ServerPropertyBase::get_all() {
    static std::vector<const ServerPropertyBase *> out;
    return out;
}
ServerPropertyBase::ServerPropertyBase() {
    get_all().push_back(this);
}

// A Single Property
template <typename T>
ServerProperty<T>::ServerProperty(const ServerProperties &properties_, const std::string &key_, const T &def_, const std::string &comment_):
    properties(properties_),
    key(key_),
    def(def_),
    comment(comment_) {}
template<typename T>
std::string ServerProperty<T>::to_string() const {
    std::string out;
    out += comment_char;
    out += ' ';
    out += comment;
    out += '\n';
    out += key;
    out += key_value_seperator;
    out += get_default_string();
    out += '\n';
    return out;
}
template <typename T>
std::optional<T> ServerProperty<T>::try_get() const {
    const std::optional<std::string> str = properties.get(key);
    if (str.has_value()) {
        return parse(str.value());
    } else {
        return std::nullopt;
    }
}
template <typename T>
T ServerProperty<T>::get() const {
    const std::optional<T> ret = try_get();
    return ret.value_or(def);
}

// Parsing
static constexpr const char *true_str = "true";
static constexpr const char *false_str = "false";
template <>
std::optional<bool> ServerProperty<bool>::parse(const std::string &str) const {
    return str == true_str || str == "yes" || str == "1";
}
template <>
std::string ServerProperty<bool>::get_default_string() const {
    return def ? true_str : false_str;
}
template <>
std::optional<int> ServerProperty<int>::parse(const std::string &str) const {
    std::string trimmed_str = str;
    trim(trimmed_str);
    if (trimmed_str.empty()) {
        return std::nullopt;
    }
    try {
        return std::stoi(trimmed_str);
    } catch (...) {
        return std::nullopt;
    }
}
template <>
std::string ServerProperty<int>::get_default_string() const {
    return safe_to_string(def);
}
template <>
std::optional<std::string> ServerProperty<std::string>::parse(const std::string &str) const {
    return str;
}
template <>
std::string ServerProperty<std::string>::get_default_string() const {
    return def;
}

// Template Installation
template struct ServerProperty<bool>;
template struct ServerProperty<int>;
template struct ServerProperty<std::string>;