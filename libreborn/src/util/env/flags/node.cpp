#include <algorithm>

#include <libreborn/log.h>
#include <libreborn/env/flags.h>

// Flag
static int next_id;
void FlagNode::reset_id_counter() {
    next_id = 1;
}
FlagNode::FlagNode(const std::string &name_) {
    name = name_;
    id = next_id++;
}
FlagNode::FlagNode():
    FlagNode("Root") {}
void FlagNode::sort() {
    // Sort
    std::ranges::sort(children, [](const FlagNode &a, const FlagNode &b) {
        // Place Categories Before Flags
        const bool a_is_category = a.is_category();
        const bool b_is_category = b.is_category();
        if (a_is_category != b_is_category) {
            return b_is_category < a_is_category;
        } else {
            // Sort Alphabetically
            return a.name < b.name;
        }
    });
    // Recurse
    for (FlagNode &child : children) {
        child.sort();
    }
}
bool FlagNode::is_category() const {
    return !value.has_value();
}

// Iteration
void FlagNode::for_each(const std::function<void(FlagNode &)> &callback) {
    for (FlagNode &child : children) {
        if (!child.is_category()) {
            callback(child);
        } else {
            child.for_each(callback);
        }
    }
}
void FlagNode::for_each_const(const std::function<void(const FlagNode &)> &callback) const {
    const_cast<FlagNode &>(*this).for_each(callback);
}

// Parsing
bool FlagNode::handle_line_prefix(const std::string &prefix, std::string &line) {
    const std::string full_prefix = prefix + ' ';
    if (line.starts_with(full_prefix)) {
        line = line.substr(full_prefix.size());
        return true;
    } else {
        return false;
    }
}
std::unordered_map<std::string, bool> FlagNode::flag_prefixes = {
    {"TRUE", true},
    {"FALSE", false}
};
FlagNode &FlagNode::add_flag(std::string line) {
    // Parse
    std::optional<bool> new_value;
    for (const std::pair<const std::string, bool> &it : flag_prefixes) {
        if (handle_line_prefix(it.first, line)) {
            new_value = it.second;
            break;
        }
    }
    // Check
    if (!new_value.has_value()) {
        ERR("Invalid Feature Flag Line: %s", line.c_str());
    }
    if (line.find(FLAG_SEPERATOR_CHAR) != std::string::npos) {
        ERR("Feature Flag Contains Invalid Character");
    }
    // Create
    FlagNode out(line);
    out.value = new_value;
    children.push_back(out);
    return children.back();
}
FlagNode &FlagNode::add_category(const std::string &new_name) {
    const FlagNode out(new_name);
    children.push_back(out);
    return children.back();
}