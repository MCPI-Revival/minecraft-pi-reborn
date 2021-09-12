#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MCPI_SERVER_MODE
void chat_open();
unsigned int chat_get_counter();
#endif // #ifndef MCPI_SERVER_MODE

__attribute__((visibility("internal"))) extern int _chat_enabled;
#ifndef MCPI_SERVER_MODE
__attribute__((visibility("internal"))) void _chat_queue_message(char *message);
#endif // #ifndef MCPI_SERVER_MODE

#ifdef __cplusplus
}
#endif
