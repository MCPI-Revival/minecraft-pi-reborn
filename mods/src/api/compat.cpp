#include <algorithm>

#include <libreborn/util/string.h>
#include <libreborn/log.h>

#include <mods/misc/misc.h>

#include "internal.h"

// Compatibility Mode
bool api_compat_mode = true;

// Read String Input
std::string api_get_input(std::string message) {
    // Decode
    if (!api_compat_mode) {
        message = misc_base64_decode(message);
    }
    // Convert To CP-437
    return to_cp437(message);
}
// Output String
std::string api_get_output(std::string message, const bool replace_comma) {
    // Convert To Unicode
    message = from_cp437(message);
    // Escape Characters
    if (api_compat_mode) {
        // Output In Plaintext For RJ Compatibility
        std::ranges::replace(message, list_separator, '\\');
        if (replace_comma) {
            std::ranges::replace(message, arg_separator, '.');
        }
    } else {
        // Encode
        message = misc_base64_encode(message);
    }
    // Return
    return message;
}

// Join Strings Into Output
std::string api_join_outputs(const std::vector<std::string> &pieces, const char separator) {
    // Join
    std::string out;
    for (std::string piece : pieces) {
        // Check
        if (piece.find(separator) != std::string::npos) {
            // This Should Be Escapes
            IMPOSSIBLE();
        }
        // Remove Trailing Newline
        if (!piece.empty() && piece.back() == '\n') {
            piece.pop_back();
        }
        // Add
        out += piece + separator;
    }
    // Remove Hanging Comma
    if (!out.empty()) {
        out.pop_back();
    }
    // Return
    return out + '\n';
}

// Entity Types
static std::unordered_map<int, EntityType> modern_entity_id_mapping = {
    {93, EntityType::CHICKEN},
    {92, EntityType::COW},
    {90, EntityType::PIG},
    {91, EntityType::SHEEP},
    {54, EntityType::ZOMBIE},
    {50, EntityType::CREEPER},
    {51, EntityType::SKELETON},
    {52, EntityType::SPIDER},
    {57, EntityType::ZOMBIE_PIGMAN},
    {1, EntityType::DROPPED_ITEM},
    {20, EntityType::PRIMED_TNT},
    {21, EntityType::FALLING_SAND},
    {10, EntityType::ARROW},
    {11, EntityType::THROWN_SNOWBALL},
    {7, EntityType::THROWN_EGG},
    {9, EntityType::PAINTING}
};
void api_convert_to_outside_entity_type(int &type) {
    if (!api_compat_mode) {
        return;
    }
    // Convert To RJ-Compatible Entity Type
    for (const std::pair<const int, EntityType> &pair : modern_entity_id_mapping) {
        if (static_cast<int>(pair.second) == type) {
            type = pair.first;
        }
    }
}
void api_convert_to_mcpi_entity_type(int &type) {
    if (!api_compat_mode) {
        return;
    }
    // Convert To Native Entity Type
    if (modern_entity_id_mapping.contains(type)) {
        type = static_cast<int>(modern_entity_id_mapping[type]);
    }
}