#include <libreborn/util/util.h>

#include <symbols/Minecraft.h>
#include <symbols/ServerSideNetworkHandler.h>

#include <mods/server/server.h>

// Add Server Command
HOOK(server_get_commands, std::vector<ServerCommand> *, (Minecraft *minecraft, ServerSideNetworkHandler *server_side_network_handler)) {
    // Call Original Method
    std::vector<ServerCommand> *commands = real_server_get_commands()(minecraft, server_side_network_handler);
    // Add Command
    commands->push_back({
        .name = "greet",
        .comment = "Example Custom Command",
        .callback = [](MCPI_UNUSED const std::string &cmd) {
            INFO("Hello World!");
        }
    });
    // Return
    return commands;
}