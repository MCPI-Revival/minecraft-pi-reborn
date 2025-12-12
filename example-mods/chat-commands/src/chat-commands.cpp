#include <libreborn/util/util.h>
#include <libreborn/util/string.h>

#include <symbols/Minecraft.h>
#include <symbols/CommandServer.h>
#include <symbols/ChatPacket.h>

#include <mods/chat/chat.h>
#include <mods/misc/misc.h>

// Send UTF-8 API Command
static std::string chat_send_api_command(const Minecraft *minecraft, const std::string &str) {
    ConnectedClient client;
    client.sock = -1;
    client.str = "";
    client.lastBlockHitPoll = 0;
    CommandServer *command_server = minecraft->command_server;
    if (command_server != nullptr) {
        return command_server->parse(client, str);
    } else {
        return "";
    }
}

// The Actual Mod
HOOK(chat_handle_packet_send, void, (const Minecraft *minecraft, ChatPacket *packet)) {
    // Get Message
    std::string message = packet->message;
    if (!message.empty() && message[0] == '\\') {
        // Convert Command To UTF-8
        message = from_cp437(message);
        message = message.substr(1);
        // API Command
        Minecraft *mc = (Minecraft *) minecraft;
        Gui *gui = &mc->gui;
        std::string out = chat_send_api_command(mc, message);
        // Display Output
        if (!out.empty() && out[out.length() - 1] == '\n') {
            out.pop_back();
        }
        out = to_cp437(out);
        gui->addMessage(out);
    } else {
        // Call Original Method
        real_chat_handle_packet_send()(minecraft, packet);
    }
}