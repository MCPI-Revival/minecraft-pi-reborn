#pragma once

#include <string>

// Message Limitations
#define MAX_CHAT_MESSAGE_LENGTH 256

// Message Prefix
__attribute__((visibility("internal"))) std::string _chat_get_prefix(char *username);

// Queue Message For Sending
#ifndef MCPI_SERVER_MODE
__attribute__((visibility("internal"))) void _chat_queue_message(const char *message);
#endif

// Init Chat UI
#ifndef MCPI_HEADLESS_MODE
__attribute__((visibility("internal"))) void _init_chat_ui();
#endif
