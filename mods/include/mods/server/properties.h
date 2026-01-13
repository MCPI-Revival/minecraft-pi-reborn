#pragma once

#include <string>
#include <map>
#include <vector>
#include <optional>

// Loaded Properties
struct ServerProperties {
    void load(std::istream &stream);
    [[nodiscard]] std::optional<std::string> get(const std::string &key) const;
private:
    std::map<std::string, std::string> properties = {};
};

// A Single Property Definition
struct ServerPropertyBase {
    // Constructor
    static std::vector<const ServerPropertyBase *> &get_all();
    ServerPropertyBase();
    virtual ~ServerPropertyBase() = default;
    // Methods
    [[nodiscard]] virtual std::string to_string() const = 0;
};
template <typename T>
struct ServerProperty : ServerPropertyBase {
    // Constructor
    ServerProperty(const ServerProperties &properties_, const std::string &key_, const T &def_, const std::string &comment_);
    // Properties
    const ServerProperties &properties;
    const std::string key;
    const T def;
    const std::string comment;
    // Methods
    [[nodiscard]] std::string to_string() const override;
    [[nodiscard]] T get() const;
    [[nodiscard]] std::optional<T> try_get() const;
private:
    [[nodiscard]] std::string get_default_string() const;
    [[nodiscard]] std::optional<T> parse(const std::string &str) const;
};