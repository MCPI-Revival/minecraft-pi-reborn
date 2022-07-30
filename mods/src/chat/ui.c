// Config Needs To Load First
#include <libreborn/libreborn.h>

// Chat UI Code Is Useless In Headless Mode
#ifndef MCPI_SERVER_MODE

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include <media-layer/core.h>

#include "chat-internal.h"
#include <mods/chat/chat.h>

// Count Chat Windows
static pthread_mutex_t chat_counter_lock = PTHREAD_MUTEX_INITIALIZER;
static volatile unsigned int chat_counter = 0;
unsigned int chat_get_counter() {
    return chat_counter;
}

// Chat Thread
#define DIALOG_TITLE "Chat"
static void *chat_thread(__attribute__((unused)) void *nop) {
    // Open
    int return_code;
    const char *command[] = {
        "zenity",
        "--title", DIALOG_TITLE,
        "--name", MCPI_APP_ID,
        "--entry",
        "--text", "Enter Chat Message:",
        NULL
    };
    char *output = run_command(command, &return_code);
    // Handle Message
    if (output != NULL) {
        // Check Return Code
        if (is_exit_status_success(return_code)) {
            // Remove Ending Newline
            int length = strlen(output);
            if (output[length - 1] == '\n') {
                output[length - 1] = '\0';
            }
            length = strlen(output);
            // Don't Allow Empty Strings
            if (length > 0) {
                // Submit
                char *safe_output = to_cp437(output);
                _chat_queue_message(safe_output);
                free(safe_output);
            }
        }
        // Free Output
        free(output);
    }
    // Update Counter
    pthread_mutex_lock(&chat_counter_lock);
    chat_counter--;
    pthread_mutex_unlock(&chat_counter_lock);
    // Return
    return NULL;
}

// Create Chat Thead
void chat_open() {
    if (_chat_enabled) {
        // Lock UI
        media_set_interactable(0);

        // Update Counter
        pthread_mutex_lock(&chat_counter_lock);
        chat_counter++;
        pthread_mutex_unlock(&chat_counter_lock);
        // Start Thread
        pthread_t thread;
        pthread_create(&thread, NULL, chat_thread, NULL);
    }
}

#endif
