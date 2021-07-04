#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void chat_open();
unsigned int chat_get_counter();

__attribute__((visibility("internal"))) extern int _chat_enabled;
__attribute__((visibility("internal"))) void _chat_queue_message(char *message);

#ifdef __cplusplus
}
#endif
