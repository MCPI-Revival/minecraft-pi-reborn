#include <string>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "misc-internal.h"
#include <mods/misc/misc.h>

// Print Chat To Log
static bool Gui_addMessage_recursing = false;
static void Gui_addMessage_injection(Gui_addMessage_t original, Gui *gui, const std::string &text) {
    // Sanitize Message
    char *new_message = strdup(text.c_str());
    ALLOC_CHECK(new_message);
    sanitize_string(new_message, -1, 1);
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
        original(gui, cpp_str);

        // End Recursing
        Gui_addMessage_recursing = false;
    } else {
        // Call Original Method
        original(gui, cpp_str);
    }

    // Free
    free(new_message);
}

// Print Progress Reports
static int last_progress = -1;
static const char *last_message = nullptr;
static void print_progress(Minecraft *minecraft) {
    const char *message = minecraft->getProgressMessage();
    int32_t progress = minecraft->progress;
    if (minecraft->isLevelGenerated()) {
        message = "Ready";
        progress = -1;
    }
    if (message != nullptr) {
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
static void Level_saveLevelData_injection(Level_saveLevelData_t original, Level *level) {
    // Print Log Message
    DEBUG("Saving Game");

    // Call Original Method
    original(level);
}

// Init
void _init_misc_logging() {
    // Print Chat To Log
    overwrite_calls(Gui_addMessage, Gui_addMessage_injection);

    // Print Progress Reports
    misc_run_on_update(Minecraft_update_injection);

    // Print Log On Game Save
    overwrite_calls(Level_saveLevelData, Level_saveLevelData_injection);

    // Disable stdout Buffering
    setvbuf(stdout, nullptr, _IONBF, 0);
}
