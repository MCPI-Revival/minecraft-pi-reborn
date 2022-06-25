#pragma once

#include <libreborn/libreborn.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MCPI_SERVER_MODE
void chat_open();
unsigned int chat_get_counter();
#endif

// Override using the HOOK() macro to provide customized chat behavior.
void chat_send_message(unsigned char *server_side_network_handler, char *username, char *message);

#ifdef __cplusplus
}
#endif
