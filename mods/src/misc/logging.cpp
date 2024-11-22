#include <string>

#include <libreborn/patch.h>
#include <libreborn/string.h>
#include <libreborn/util.h>

#include <symbols/minecraft.h>

#include "misc-internal.h"
#include <mods/misc/misc.h>
#include <mods/feature/feature.h>

// Print Chat To Log
static void Gui_addMessage_injection(Gui_addMessage_t original, Gui *gui, const std::string &text) {
    // Sanitize Message
    std::string new_message = text;
    sanitize_string(new_message, -1, true);

    // Process Message
    static bool recursing = false;
    if (!recursing) {
        // Start Recursing
        recursing = true;

        // Print Log Message
        std::string safe_message = from_cp437(new_message);
        fprintf(stderr, "[CHAT]: %s\n", safe_message.c_str());

        // Call Original Method
        original(gui, new_message);

        // End Recursing
        recursing = false;
    } else {
        // Call Original Method
        original(gui, new_message);
    }
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
        const bool message_different = message != last_message;
        const bool progress_significant = is_progress_difference_significant(progress, last_progress);
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
    if (feature_has("Log Chat Messages", server_enabled)) {
        overwrite_calls(Gui_addMessage, Gui_addMessage_injection);
    }

    // Game Status
    if (feature_has("Log Game Status", server_enabled)) {
        // Print Progress Reports
        misc_run_on_update(Minecraft_update_injection);

        // Print Log On Game Save
        overwrite_calls(Level_saveLevelData, Level_saveLevelData_injection);
    }

    // Disable stdout Buffering
    setvbuf(stdout, nullptr, _IONBF, 0);
}
