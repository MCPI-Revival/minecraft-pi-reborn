#include <string>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "misc-internal.h"
#include <mods/misc/misc.h>

// Print Chat To Log
static bool Gui_addMessage_recursing = false;
static void Gui_addMessage_injection(Gui *gui, std::string *text) {
    // Sanitize Message
    char *new_message = strdup(text->c_str());
    ALLOC_CHECK(new_message);
    sanitize_string(&new_message, -1, 1);
    std::string cpp_str = new_message;

    // Process Message
    if (!Gui_addMessage_recursing) {
        // Start Recursing
        Gui_addMessage_recursing = true;

        // Print Log Message
        char *safe_message = from_cp437(new_message);
        fprintf(stderr, "[CHAT]: %s\n", safe_message);
        free(safe_message);

        // Call Original Method
        (*Gui_addMessage)(gui, &cpp_str);

        // End Recursing
        Gui_addMessage_recursing = false;
    } else {
        // Call Original Method
        (*Gui_addMessage)(gui, &cpp_str);
    }

    // Free
    free(new_message);
}
void misc_add_message(Gui *gui, const char *text) {
    std::string str = text;
    Gui_addMessage_injection(gui, &str);
}

// Print Progress Reports
static int last_progress = -1;
static const char *last_message = NULL;
static void print_progress(Minecraft *minecraft) {
    const char *message = (*Minecraft_getProgressMessage)(minecraft);
    int32_t progress = minecraft->progress;
    if ((*Minecraft_isLevelGenerated)(minecraft)) {
        message = "Ready";
        progress = -1;
    }
    if (message != NULL) {
        bool message_different = message != last_message;
        bool progress_significant = is_progress_difference_significant(progress, last_progress);
        if (message_different || progress_significant) {
            if (progress != -1) {
                INFO("Status: %s: %i%%", message, progress);
            } else {
                INFO("Status: %s", message);
            }
            if (message_different) {
                last_message = message;
            }
            if (progress_significant) {
                last_progress = progress;
            }
        }
    }
}

// Print Progress Reports Regularly
static void Minecraft_update_injection(Minecraft *minecraft) {
    // Print Progress Reports
    print_progress(minecraft);
}

// Log When Game Is Saved
void Level_saveLevelData_injection(Level *level) {
    // Print Log Message
    DEBUG("Saving Game");

    // Call Original Method
    (*Level_saveLevelData)(level);
}

// Init
void _init_misc_logging() {
    // Print Chat To Log
    overwrite_calls((void *) Gui_addMessage, (void *) Gui_addMessage_injection);

    // Print Progress Reports
    misc_run_on_update(Minecraft_update_injection);

    // Print Log On Game Save
    overwrite_calls((void *) Level_saveLevelData, (void *) Level_saveLevelData_injection);

    // Disable stdout Buffering
    setvbuf(stdout, NULL, _IONBF, 0);
}
