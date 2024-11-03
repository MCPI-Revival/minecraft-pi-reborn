#pragma once

#include <string>

#include <symbols/minecraft.h>

// Message Limitations
#define MAX_CHAT_MESSAGE_LENGTH 256

// Message Prefix
__attribute__((visibility("internal"))) std::string _chat_get_prefix(const char *username);
// Queue Message For Sending
__attribute__((visibility("internal"))) void _chat_send_message_to_server(const Minecraft *minecraft, const char *message);
// Init Chat UI
__attribute__((visibility("internal"))) void _init_chat_ui();