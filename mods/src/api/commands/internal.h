#pragma once

#include "../internal.h"

#include <functional>
#include <optional>
#include <sstream>

// Force Semicolon After Calling Function Macro
#define force_semicolon() (void) 0

// Read Arguments
#define next_string(out, required) \
    std::string out; \
    const bool out##_missing = !std::getline(args, out, arg_separator); \
    if (out##_missing && (required)) { \
        return CommandServer::Fail; \
    } \
    force_semicolon()
#define next_number(out, type, func) \
    next_string(out##_str, true); \
    type out; \
    try { \
        (out) = func(out##_str); \
    } catch (...) { \
        return CommandServer::Fail; \
    } \
    force_semicolon()
#define next_int(out) next_number(out, int, std::stoi)
#define next_float(out) next_number(out, float, std::stof)

// Parse Commands
#define package_str(name) (#name ".")
MCPI_UNUSED static bool _package(std::string_view &cmd, const std::string &package) {
    if (cmd.starts_with(package)) {
        cmd.remove_prefix(package.size());
        return true;
    } else {
        return false;
    }
}
#define package(name) if (_package(cmd, package_str(name)))
#define command(name) if (cmd == #name)
#define passthrough(name) \
    command(name) { \
        return super(); \
    } \
    force_semicolon()

// Packages
struct CommandServer;
struct ConnectedClient;
MCPI_INTERNAL std::string api_handle_command(const std::function<std::string()> &super, CommandServer *server, const ConnectedClient &client, std::string_view &cmd, std::istringstream &args);
MCPI_INTERNAL std::string api_handle_world_command(const std::function<std::string()> &super, CommandServer *server, std::string_view &cmd, std::istringstream &args);
MCPI_INTERNAL std::string api_handle_entity_command(const std::function<std::string()> &super, CommandServer *server, const ConnectedClient &client, std::string_view &cmd, std::istringstream &args);
MCPI_INTERNAL std::string api_handle_event_command(CommandServer *server, const ConnectedClient &client, std::string_view &cmd, const std::optional<int> &id);
MCPI_INTERNAL void _init_api_events();

// Utility Functions
struct Entity;
MCPI_INTERNAL std::string api_get_entity_message(CommandServer *server, Entity *entity);
MCPI_INTERNAL bool api_is_entity_selected(const Entity *entity, int target_type);

// Control "Null Responses"
MCPI_INTERNAL extern bool api_replace_null_responses;