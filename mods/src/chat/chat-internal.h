#pragma once

#include <libreborn/libreborn.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MCPI_SERVER_MODE
__attribute__((visibility("internal"))) void _chat_queue_message(const char *message);
#endif

#ifndef MCPI_HEADLESS_MODE
__attribute__((visibility("internal"))) void _init_chat_ui();
#endif

#ifdef __cplusplus
}
#endif
