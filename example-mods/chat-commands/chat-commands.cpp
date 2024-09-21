// Headers
#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <mods/chat/chat.h>
#include <mods/misc/misc.h>
#include <mods/server/server.h>

// The Actual Mod
HOOK(chat_handle_packet_send, void, (const Minecraft *minecraft, ChatPacket *packet)) {
    // Get Message
    const char *message = packet->message.c_str();
    if (message[0] == '/') {
        // API Command
        Minecraft *mc = (Minecraft *) minecraft;
        Gui *gui = &mc->gui;
        std::string out = chat_send_api_command(mc, (char *) &message[1]);
        if (out.length() > 0 && out[out.length() - 1] == '\n') {
            out[out.length() - 1] = '\0';
        }
        gui->addMessage(out);
    } else {
        // Call Original Method
        ensure_chat_handle_packet_send();
        real_chat_handle_packet_send(minecraft, packet);
    }
}

// Add Server Command
HOOK(server_get_commands, std::vector<ServerCommand> *, (Minecraft *minecraft, ServerSideNetworkHandler *server_side_network_handler)) {
    // Call Original Method
    ensure_server_get_commands();
    std::vector<ServerCommand> *commands = real_server_get_commands(minecraft, server_side_network_handler);
    // Add Command
    commands->push_back({
        .name = "greet",
        .comment = "Example Custom Command",
        .callback = [](__attribute__((unused)) const std::string &cmd) {
            INFO("Hello World!");
        }
    });
    // Return
    return commands;
}