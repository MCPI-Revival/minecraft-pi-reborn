// Headers
#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <mods/chat/chat.h>
#include <mods/misc/misc.h>

// The Actual Mod
HOOK(chat_handle_packet_send, void, (Minecraft *minecraft, ChatPacket *packet)) {
    // Get Message
    const char *message = packet->message.c_str();
    if (message[0] == '/') {
        // API Command
        Gui *gui = &minecraft->gui;
        std::string out = chat_send_api_command(minecraft, (char *) &message[1]);
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
