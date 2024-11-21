#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

// Seperator
#define SEPERATOR_CHAR '|'

// Flag
struct FlagNode {
private:
    explicit FlagNode(const std::string &name_);
public:
    FlagNode();
    // Methods
    void sort();
    void for_each(const std::function<void(FlagNode &)> &callback);
    void for_each_const(const std::function<void(const FlagNode &)> &callback) const;
    void add_flag(std::string line);
    FlagNode &add_category(const std::string &new_name);
    // Properties
    std::string name;
    bool value;
    std::vector<FlagNode> children;
    int id;
    // Internal
    static bool handle_line_prefix(const std::string &prefix, std::string &line);
    static std::unordered_map<std::string, bool> flag_prefixes;
    static void reset_id_counter();
};

// All Flags
struct Flags {
    explicit Flags(const std::string &data);
    static Flags get();
    // To/From Strings
    explicit operator std::string() const;
    Flags &operator=(const std::string &str);
    // To/From Cache
    [[nodiscard]] std::unordered_map<std::string, bool> to_cache() const;
    void from_cache(const std::unordered_map<std::string, bool> &cache);
    // Print
    void print() const;
    // Properties
    FlagNode root;
};