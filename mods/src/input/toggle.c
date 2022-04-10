#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "input.h"
#include "../feature/feature.h"

// Enable Toggles
static int enable_toggles = 0;

// Store Function Input
static int hide_gui_toggle = 0;
void input_hide_gui() {
    hide_gui_toggle++;
}
static int third_person_toggle = 0;
void input_third_person() {
    third_person_toggle++;
}

// Handle Toggle Options
static void _handle_toggle_options(unsigned char *minecraft) {
    if (enable_toggles) {
        // Handle Functions
        unsigned char *options = minecraft + Minecraft_options_property_offset;
        if (hide_gui_toggle % 2 != 0) {
            // Toggle Hide GUI
            *(options + Options_hide_gui_property_offset) = *(options + Options_hide_gui_property_offset) ^ 1;
        }
        hide_gui_toggle = 0;
        if (third_person_toggle % 2 != 0) {
            // Toggle Third Person
            *(options + Options_third_person_property_offset) = *(options + Options_third_person_property_offset) ^ 1;
        }
        third_person_toggle = 0;
    }
}

// Init
void _init_toggle() {
    enable_toggles = feature_has("Bind Common Toggleable Options To Function Keys", server_disabled);
    input_run_on_tick(_handle_toggle_options);
}
