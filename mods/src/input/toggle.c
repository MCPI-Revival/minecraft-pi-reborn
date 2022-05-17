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
            *(options + Options_third_person_property_offset) = (*(options + Options_third_person_property_offset) + 1) % 3;
        }
        third_person_toggle = 0;
        // Fix Broken Value From Third-Person OptionsButton Toggle
        // (Because Front-Facing Code Repurposes A Boolean As A Ternary)
        if (*(options + Options_third_person_property_offset) == 3) {
            *(options + Options_third_person_property_offset) = 0;
        }
    }
}

// Font-Facing View
static void GameRenderer_setupCamera_injection(unsigned char *game_renderer, float param_1, int param_2) {
    // Get Objects
    unsigned char *minecraft = *(unsigned char **) (game_renderer + GameRenderer_minecraft_property_offset);
    unsigned char *player = *(unsigned char **) (minecraft + Minecraft_player_property_offset);

    // Check If In Third-Person
    unsigned char *options = minecraft + Minecraft_options_property_offset;
    int is_font_facing = (*(options + Options_third_person_property_offset) == 2);

    // Invert Rotation
    if (is_font_facing && player != NULL) {
        *(float *) (player + Entity_yaw_property_offset) = 180.f + (*(float *) (player + Entity_yaw_property_offset));
        *(float *) (player + Entity_old_yaw_property_offset) = 180.f + (*(float *) (player + Entity_old_yaw_property_offset));
        *(float *) (player + Entity_pitch_property_offset) = -(*(float *) (player + Entity_pitch_property_offset));
        *(float *) (player + Entity_old_pitch_property_offset) = -(*(float *) (player + Entity_old_pitch_property_offset));
    }

    // Call Original Method
    (*GameRenderer_setupCamera)(game_renderer, param_1, param_2);

    // Revert
    if (is_font_facing && player != NULL) {
        *(float *) (player + Entity_yaw_property_offset) = -180.f + (*(float *) (player + Entity_yaw_property_offset));
        *(float *) (player + Entity_old_yaw_property_offset) = -180.f + (*(float *) (player + Entity_old_yaw_property_offset));
        *(float *) (player + Entity_pitch_property_offset) = -(*(float *) (player + Entity_pitch_property_offset));
        *(float *) (player + Entity_old_pitch_property_offset) = -(*(float *) (player + Entity_old_pitch_property_offset));
    }
}

// Init
void _init_toggle() {
    enable_toggles = feature_has("Bind Common Toggleable Options To Function Keys", server_disabled);
    input_run_on_tick(_handle_toggle_options);

    // Font-Facing View
    if (enable_toggles) {
        overwrite_calls((void *) GameRenderer_setupCamera, (void *) GameRenderer_setupCamera_injection);
    }
}
