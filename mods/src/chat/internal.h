#pragma once

#include <string>

#include <symbols/minecraft.h>

// Message Limitations
#define MAX_CHAT_MESSAGE_LENGTH 256

// Message Prefix
MCPI_INTERNAL std::string _chat_get_prefix(const char *username);
// Queue Message For Sending
MCPI_INTERNAL void _chat_send_message_to_server(const Minecraft *minecraft, const char *message);
// Init Chat UI
MCPI_INTERNAL void _init_chat_ui();
MCPI_INTERNAL void _chat_clear_history();