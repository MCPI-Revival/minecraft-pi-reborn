#include <algorithm>

#include <libreborn/log.h>
#include <libreborn/flags.h>

// Flag
static int next_id;
void FlagNode::reset_id_counter() {
    next_id = 1;
}
FlagNode::FlagNode(const std::string &name_) {
    name = name_;
    value = false;
    id = next_id++;
}
FlagNode::FlagNode(): FlagNode("Root") {}
void FlagNode::sort() {
    // Sort
    std::ranges::sort(children, [](const FlagNode &a, const FlagNode &b) {
        // Place Categories Before Flags
        if (a.children.empty() != b.children.empty()) {
            return a.children.empty() < b.children.empty();
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

// Iteration
void FlagNode::for_each(const std::function<void(FlagNode &)> &callback) {
    for (FlagNode &child : children) {
        if (child.children.empty()) {
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
void FlagNode::add_flag(std::string line) {
    // Parse
    bool value_set = false;
    bool new_value = false;
    for (const std::pair<const std::string, bool> &it : flag_prefixes) {
        if (handle_line_prefix(it.first, line)) {
            new_value = it.second;
            value_set = true;
            break;
        }
    }
    // Check
    if (!value_set) {
        ERR("Invalid Feature Flag Line: %s", line.c_str());
    }
    if (line.rfind(FLAG_SEPERATOR_CHAR, 0) != std::string::npos) {
        ERR("Feature Flag Contains Invalid Character");
    }
    // Create
    FlagNode out(line);
    out.value = new_value;
    children.push_back(out);
}
FlagNode &FlagNode::add_category(const std::string &new_name) {
    const FlagNode out(new_name);
    children.push_back(out);
    return children.back();
}