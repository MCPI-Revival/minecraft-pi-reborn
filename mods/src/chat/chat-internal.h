#pragma once

#include <libreborn/libreborn.h>

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((visibility("internal"))) extern int _chat_enabled;
#ifndef MCPI_SERVER_MODE
__attribute__((visibility("internal"))) void _chat_queue_message(char *message);
#endif

#ifdef __cplusplus
}
#endif
