#include <string>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>

#include "options-internal.h"

// Fix Initial Option Button Rendering
// The calling function doesn't exist in MCPE v0.6.1, so its name is unknown.
static OptionButton *OptionsPane_unknown_toggle_creating_function_OptionButton_injection(OptionButton *option_button, Options_Option *option) {
    // Call Original Method
    OptionButton *ret = OptionButton_constructor(option_button, option);

    // Setup Image
    OptionButton_updateImage(option_button, stored_options);

    // Return
    return ret;
}

// Modify Option Toggles
static void OptionsPane_unknown_toggle_creating_function_injection(OptionsPane_unknown_toggle_creating_function_t original, OptionsPane *options_pane, uint32_t group_id, std::string *name_ptr, Options_Option *option) {
    // Modify
    std::string name = *name_ptr;
    std::string new_name = name;
    if (name == "Fancy Graphics") {
        option = &Options_Option_GRAPHICS;
    } else if (name == "Soft shadows") {
        option = &Options_Option_AMBIENT_OCCLUSION;
    } else if (name == "Fancy Skies" || name == "Animated water") {
        // These have no corresponding option, so disable the toggle.
        return;
    } else if (name == "Third person camera") {
        // This isn't saved/loaded, so disable the toggle.
        return;
    } else if (name == "Lefty" || name == "Use touch screen" || name == "Split touch controls") {
        // These toggles require touch support, so disable them.
        return;
    } else if (name == "Vibrate on destroy") {
        // This toggle requires vibration support, so disable it.
        return;
    } else if (name == "Invert X-axis") {
        // Fix Incorrect Name
        new_name = "Invert Y-axis";
    }

    // Call Original Method
    original(options_pane, group_id, &new_name, option);

    // Add 3D Anaglyph
    if (option == &Options_Option_GRAPHICS) {
        std::string cpp_string = "3D Anaglyph";
        original(options_pane, group_id, &cpp_string, &Options_Option_ANAGLYPH);
    }

    // Add Peaceful Mode
    if (option == &Options_Option_SERVER_VISIBLE) {
        std::string cpp_string = "Peaceful mode";
        original(options_pane, group_id, &cpp_string, &Options_Option_DIFFICULTY);
    }
}

// Add Missing Options To Options::getBooleanValue
static bool Options_getBooleanValue_injection(Options_getBooleanValue_t original, Options *options, Options_Option *option) {
    // Check
    if (option == &Options_Option_GRAPHICS) {
        return options->fancy_graphics;
    } else if (option == &Options_Option_DIFFICULTY) {
        return options->game_difficulty == 0;
    } else {
        // Call Original Method
        return original(options, option);
    }
}

// Fix Difficulty When Toggling
static void OptionButton_toggle_Options_save_injection(Options *self) {
    // Fix Value
    if (self->game_difficulty == 1) {
        // Disable Peaceful
        self->game_difficulty = 2;
    } else if (self->game_difficulty == 3) {
        // Switch To Peaceful
        self->game_difficulty = 0;
    }
    // Call Original Method
    Options_save(self);
}

// Init
void _init_options_ui() {
    // Fix Options Screen
    unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    if (feature_has("Fix Options Screen", server_disabled)) {
        // Fix Initial Option Button Rendering
        overwrite_call((void *) 0x24510, (void *) OptionsPane_unknown_toggle_creating_function_OptionButton_injection);

        // "Gui Scale" slider is broken, so disable it.
        patch((void *) 0x35a10, nop_patch);
        // "Vibrate on destroy" is disabled, so "Feedback" is empty, so disable it.
        patch((void *) 0x35960, nop_patch);

        // Disconnect "This works?" Slider From Difficulty
        unsigned char this_works_slider_patch[4] = {0x00, 0x30, 0xa0, 0xe3}; // "mov r3, #0x0"
        patch((void *) 0x3577c, this_works_slider_patch);

        // Modify Option Toggles
        overwrite_calls(OptionsPane_unknown_toggle_creating_function, OptionsPane_unknown_toggle_creating_function_injection);

        // Add Missing Options To Options::getBooleanValue
        overwrite_calls(Options_getBooleanValue, Options_getBooleanValue_injection);

        // Fix Difficulty When Toggling
        overwrite_call((void *) 0x1cd00, (void *) OptionButton_toggle_Options_save_injection);
    }
}