#include <sstream>
#include <algorithm>
#include <unordered_set>

#include <libreborn/libreborn.h>

#include "flags.h"

// All Flags
static unsigned int find_indent_level(std::string &str) {
    constexpr unsigned int INDENT = 4;
    unsigned int count = 0;
    for (const char c : str) {
        if (c == ' ') {
            count++;
        } else {
            break;
        }
    }
    str = str.substr(count);
    return count / INDENT;
}
Flags::Flags(const std::string &data) {
    // Read Lines
    std::stringstream stream(data);
    std::string line;
    std::vector stack = {&root};
    while (std::getline(stream, line)) {
        if (!line.empty()) {
            // Get Parent
            const unsigned int indent = find_indent_level(line);
            if (indent >= stack.size()) {
                ERR("Bad Feature Flag Indent: %s", line.c_str());
            }
            stack.resize(indent + 1);
            FlagNode &parent = *stack.back();
            // Add New Node
            if (FlagNode::handle_line_prefix("CATEGORY", line)) {
                // New Category
                FlagNode &category = parent.add_category(line);
                stack.push_back(&category);
            } else {
                // Add Flag
                if (indent == 0) {
                    ERR("Feature Flag Outside Of Category: %s", line.c_str());
                }
                parent.add_flag(line);
            }
        }
    }
    // Sort
    root.sort();
}
Flags::operator std::string() const {
    std::string out;
    root.for_each_const([&out](const FlagNode &flag) {
        if (flag.value) {
            if (!out.empty()) {
                out += SEPERATOR_CHAR;
            }
            out += flag.name;
        }
    });
    return out;
}
Flags &Flags::operator=(const std::string &str) {
    // Find Flags To Enable
    std::unordered_set<std::string> to_enable;
    std::stringstream stream(str);
    std::string flag_name;
    while (std::getline(stream, flag_name, SEPERATOR_CHAR)) {
        if (!flag_name.empty()) {
            to_enable.insert(flag_name);
        }
    }
    // Update Flags
    root.for_each([&to_enable](FlagNode &flag) {
        flag.value = to_enable.contains(flag.name);
    });
    return *this;
}
std::unordered_map<std::string, bool> Flags::to_cache() const {
    std::unordered_map<std::string, bool> out;
    root.for_each_const([&out](const FlagNode &flag) {
        out[flag.name] = flag.value;
    });
    return out;
}
void Flags::from_cache(const std::unordered_map<std::string, bool> &cache) {
    root.for_each([&cache](FlagNode &flag) {
        if (cache.contains(flag.name)) {
            flag.value = cache.at(flag.name);
        }
    });
}
void Flags::print() const {
    root.for_each_const([](const FlagNode &flag) {
        std::string prefix;
        for (const std::pair<const std::string, bool> &it : FlagNode::flag_prefixes) {
            if (it.second == flag.value) {
                prefix = it.first;
                break;
            }
        }
        if (prefix.empty()) {
            IMPOSSIBLE();
        }
        printf("%s %s\n", prefix.c_str(), flag.name.c_str());
        fflush(stdout);
    });
}

// Instance
EMBEDDED_RESOURCE(available_feature_flags);
Flags Flags::get() {
    return Flags(std::string(available_feature_flags, available_feature_flags + available_feature_flags_len));
}