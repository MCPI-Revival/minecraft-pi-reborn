#pragma once

#include <string>

#include <symbols/Minecraft.h>

// Message Limitations
#define MAX_CHAT_MESSAGE_LENGTH 256

// Message Prefix
std::string _chat_get_prefix(const char *username);
// Queue Message For Sending
void _chat_send_message_to_server(const Minecraft *minecraft, const char *message);
// Init Chat UI
void _init_chat_ui();
void _chat_clear_history();