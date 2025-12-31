#pragma once

#include <string>
#include <deque>
#include <unordered_map>
#include <functional>
#include <optional>

// Seperator
#define FLAG_SEPERATOR_CHAR '|'

// Flag
class FlagNode {
    // Constructors
    explicit FlagNode(const std::string &name_);
public:
    FlagNode();

    // Methods
    void sort();
    void for_each(const std::function<void(FlagNode &)> &callback);
    void for_each_const(const std::function<void(const FlagNode &)> &callback) const;
    FlagNode &add_flag(std::string line);
    FlagNode &add_category(const std::string &new_name);
    [[nodiscard]] bool is_category() const;

    // Properties
    std::string name;
    std::string comment;
    std::optional<bool> value;
    std::deque<FlagNode> children; // Used Instead Of std::vector To Keep References Stable
    int id;

    // Internal
    static bool handle_line_prefix(const std::string &prefix, std::string &line);
    static std::unordered_map<std::string, bool> flag_prefixes;
    static void reset_id_counter();
};

// All Flags
struct Flags {
    // Constructor
    explicit Flags(const std::string &data);
    static Flags get();

    // To/From Strings
    [[nodiscard]] std::string to_string() const;
    void from_string(const std::string &str);
    bool operator==(const Flags &other) const;

    // To/From Cache
    [[nodiscard]] std::unordered_map<std::string, bool> to_cache() const;
    void from_cache(const std::unordered_map<std::string, bool> &cache);

    // Print
    void print() const;

    // Properties
    FlagNode root;
};