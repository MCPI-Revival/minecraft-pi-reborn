#ifndef MCPI_SERVER_MODE
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include <libreborn/libreborn.h>

#include "chat.h"
#include "../input/input.h"

// Run Command
static char *run_command_proper(const char *command[], int *return_code) {
    // Prepare Environment
    RESET_ENVIRONMENTAL_VARIABLE("LD_LIBRARY_PATH");
    RESET_ENVIRONMENTAL_VARIABLE("LD_PRELOAD");

    // Run
    return run_command(command, return_code);
}

// Count Chat Windows
static pthread_mutex_t chat_counter_lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned int chat_counter = 0;
unsigned int chat_get_counter() {
    return chat_counter;
}

// Chat Thread
static void *chat_thread(__attribute__((unused)) void *nop) {
    // Open
    int return_code;
    const char *command[] = {"zenity", "--title", "Chat", "--class", GUI_TITLE, "--entry", "--text", "Enter Chat Message:", NULL};
    char *output = run_command_proper(command, &return_code);
    // Handle Message
    if (output != NULL) {
        // Check Return Code
        if (return_code == 0) {
            // Remove Ending Newline
            int length = strlen(output);
            if (output[length - 1] == '\n') {
                output[length - 1] = '\0';
            }
            length = strlen(output);
            // Don't Allow Empty Strings
            if (length > 0) {
                // Submit
                _chat_queue_message(output);
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
        // Release Mouse
        input_set_mouse_grab_state(1);

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
