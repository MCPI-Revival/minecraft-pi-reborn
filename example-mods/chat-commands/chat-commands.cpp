// Headers

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <mods/chat/chat.h>
#include <mods/misc/misc.h>

// The Actual Mod
HOOK(chat_handle_packet_send, void, (unsigned char *minecraft, unsigned char *packet)) {
    // Get Message
    char *message = *(char **) (packet + ChatPacket_message_property_offset);
    if (message[0] == '/') {
        // API Command
        unsigned char *gui = minecraft + Minecraft_gui_property_offset;
        std::string out = chat_send_api_command(minecraft, &message[1]);
        if (out.length() > 0 && out[out.length() - 1] == '\n') {
            out[out.length() - 1] = '\0';
        }
        misc_add_message(gui, out.c_str());
    } else {
        // Call Original Method
        ensure_chat_handle_packet_send();
        (*real_chat_handle_packet_send)(minecraft, packet);
    }
}
