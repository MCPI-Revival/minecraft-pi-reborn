#pragma once

#include <functional>

#include "properties.h"

struct MCPI_MODS_PUBLIC ServerCommand {
    const std::string name;
    const std::string comment;
    const std::function<void(const std::string &)> callback;
    [[nodiscard]] bool has_args() const;
    [[nodiscard]] std::string get_lhs_help() const;
    [[nodiscard]] std::string get_full_help(int max_lhs_length) const;
};

struct Minecraft;
struct ServerSideNetworkHandler;
struct ServerPlayer;

MCPI_MODS_PUBLIC ServerProperties &get_server_properties();
extern "C" {
MCPI_MODS_PUBLIC std::vector<ServerCommand> *server_get_commands(Minecraft *minecraft, ServerSideNetworkHandler *server_side_network_handler);
MCPI_MODS_PUBLIC void server_kick(const ServerPlayer *player);
}