#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include <libreborn/libreborn.h>

#include "chat.h"

#define CHAT_WINDOW_TCL \
    "set message \"\"\n" \
    "proc submit {} {\n" \
        "global message\n" \
        "puts \"$message\"\n" \
        "exit\n" \
    "}\n" \
    \
    "wm resizable . false false\n" \
    "wm title . \"Chat\"\n" \
    "wm attributes . -topmost true -type {dialog}\n" \
    \
    "ttk::label .label -text \"Enter Chat Message:\"\n" \
    \
    "ttk::entry .entry -textvariable message\n" \
    "focus .entry\n" \
    "bind .entry <Key-Return> submit\n" \
    \
    "ttk::frame .button\n" \
    "ttk::button .button.submit -text \"Submit\" -command submit\n" \
    "ttk::button .button.cancel -text \"Cancel\" -command exit\n" \
    \
    "grid .label -row 0 -padx 6 -pady 6\n" \
    "grid .entry -row 1 -padx 6\n" \
    "grid .button -row 2 -padx 3 -pady 6\n" \
    "grid .button.cancel -row 0 -column 0 -padx 3\n" \
    "grid .button.submit -row 0 -column 1 -padx 3\n"

// Run Command
static char *run_command(char *command, int *return_code) {
    // Don't Contaminate Child Process
    unsetenv("LD_LIBRARY_PATH");
    unsetenv("LD_PRELOAD");

    // Start
    FILE *out = popen(command, "r");
    if (!out) {
        ERR("%s", "Failed To Run Command");
    }

    // Record
    char *output = NULL;
    int c;
    while ((c = fgetc(out)) != EOF) {
        asprintf(&output, "%s%c", output == NULL ? "" : output, (char) c);
        ALLOC_CHECK(output);
    }

    // Return
    *return_code = pclose(out);
    return output;
}

// Count Chat Windows
static pthread_mutex_t chat_counter_lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned int chat_counter = 0;
unsigned int chat_get_counter() {
    return chat_counter;
}

// Chat Thread
static void *chat_thread(__attribute__((unused)) void *nop) {
    // Prepare
    setenv("CHAT_WINDOW_TCL", CHAT_WINDOW_TCL, 1);
    // Open
    int return_code;
    char *output = run_command("echo \"${CHAT_WINDOW_TCL}\" | wish -name \"Minecraft - Pi edition\"", &return_code);
    // Handle Message
    if (output != NULL) {
        if (return_code == 0) {
            // Remove Ending Newline
            int length = strlen(output);
            if (output[length - 1] == '\n') {
                output[length - 1] = '\0';
            }
            length = strlen(output);
            // Submit
            chat_queue_message(output);
        }
        // Free
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
    // Update Counter
    pthread_mutex_lock(&chat_counter_lock);
    chat_counter++;
    pthread_mutex_unlock(&chat_counter_lock);
    // Start Thread
    pthread_t thread;
    pthread_create(&thread, NULL, chat_thread, NULL);
}