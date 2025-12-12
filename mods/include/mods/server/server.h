#pragma once

#include <functional>

#include "properties.h"

struct ServerCommand {
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

ServerProperties &get_server_properties();
extern "C" {
std::vector<ServerCommand> *server_get_commands(Minecraft *minecraft, ServerSideNetworkHandler *server_side_network_handler);
void server_kick(const ServerPlayer *player);
}