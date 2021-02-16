#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void chat_open();
unsigned int chat_get_counter();

void chat_queue_message(char *message);
void chat_send_messages(unsigned char *minecraft);

#ifdef __cplusplus
}
#endif