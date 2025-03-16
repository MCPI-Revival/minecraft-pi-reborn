#include <string>
#include <cstdint>

#include <libreborn/patch.h>
#include <libreborn/util/exec.h>

#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/options/options.h>

#include "internal.h"

// Fix Initial Option Button Rendering
// The calling function doesn't exist in MCPE v0.6.1, so its name is unknown.
static OptionButton *OptionsPane_unknown_toggle_creating_function_OptionButton_injection(OptionButton *option_button, const Options_Option *option) {
    // Call Original Method
    OptionButton *ret = option_button->constructor(option);

    // Setup Image
    option_button->updateImage(stored_options);

    // Return
    return ret;
}

// Modify Option Toggles
static void OptionsPane_unknown_toggle_creating_function_injection(OptionsPane_unknown_toggle_creating_function_t original, OptionsPane *options_pane, const uint32_t group_id, std::string *name_ptr, Options_Option *option) {
    // Modify
    const std::string name = *name_ptr;
    std::string new_name = name;
    if (name == "Fancy Graphics") {
        option = &Options_Option::GRAPHICS;
    } else if (name == "Soft shadows") {
        option = &Options_Option::AMBIENT_OCCLUSION;
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
    if (option == &Options_Option::GRAPHICS) {
        std::string cpp_string = "3D Anaglyph";
        original(options_pane, group_id, &cpp_string, &Options_Option::ANAGLYPH);
    }

    // Add Peaceful Mode
    if (option == &Options_Option::SERVER_VISIBLE) {
        std::string cpp_string = "Peaceful mode";
        original(options_pane, group_id, &cpp_string, &Options_Option::DIFFICULTY);
    }
}

// Add Missing Options To Options::getBooleanValue
static bool Options_getBooleanValue_injection(Options_getBooleanValue_t original, Options *options, Options_Option *option) {
    // Check
    if (option == &Options_Option::GRAPHICS) {
        return options->fancy_graphics;
    } else if (option == &Options_Option::DIFFICULTY) {
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
    self->save();
}

// Add "Reborn" Info Button
#define INFO_BUTTON_ID 99
static void OptionsScreen_init_injection(OptionsScreen_init_t original, OptionsScreen *self) {
    // Call Original Method
    original(self);

    // Add Button
    Touch_TButton *button = Touch_TButton::allocate();
    std::string name = "Reborn";
    button->constructor(INFO_BUTTON_ID, name);
    self->rendered_buttons.push_back((Button *) button);
    self->selectable_buttons.push_back((Button *) button);
}
static void OptionsScreen_setupPositions_injection(OptionsScreen_setupPositions_t original, OptionsScreen *self) {
    // Call Original Method
    original(self);

    // Find Button
    const Button *prevButton = nullptr;
    Button *button = nullptr;
    for (Button *x : self->selectable_buttons) {
        if (x->id == INFO_BUTTON_ID) {
            button = x;
            break;
        }
        prevButton = x;
    }
    if (button == nullptr || prevButton == nullptr) {
        IMPOSSIBLE();
    }

    // Setup Button
    button->width = prevButton->width;
    button->height = prevButton->height;
    button->x = prevButton->x;
    button->y = prevButton->y + prevButton->height;
}
static void OptionsScreen_buttonClicked_injection(OptionsScreen_buttonClicked_t original, OptionsScreen *self, Button *button) {
    // Check ID
    if (button->id == INFO_BUTTON_ID) {
        // Show Screen
        self->minecraft->setScreen(_create_options_info_screen());
    } else {
        // Call Original Method
        original(self, button);
    }
}
static void OptionsScreen_removed_injection(OptionsScreen_removed_t original, OptionsScreen *self) {
    // Delete Button
    Button *button = nullptr;
    for (Button *x : self->selectable_buttons) {
        if (x->id == INFO_BUTTON_ID) {
            button = x;
            break;
        }
    }
    if (button == nullptr) {
        IMPOSSIBLE();
    }
    button->destructor_deleting();

    // Call Original Method
    original(self);
}

// Show Documentation About Sound Data
static void OptionButton_toggle_injection(OptionButton_toggle_t original, OptionButton *self, Options *options) {
    if (self->option == &Options_Option::SOUND && info_sound_data_state != info_sound_data_loaded) {
        // Open
        open_url(SOUND_DOC_URL);
    } else {
        // Call Original Method
        original(self, options);
    }
}

// Init
void _init_options_ui() {
    // Fix Options Screen
    unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    if (feature_has("Fix Options Screen", server_disabled)) {
        // Fix Initial Option Button Rendering
        overwrite_call((void *) 0x24510, OptionButton_constructor, OptionsPane_unknown_toggle_creating_function_OptionButton_injection);

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
        overwrite_call((void *) 0x1cd00, Options_save, OptionButton_toggle_Options_save_injection);

        // Show Documentation When User Attempts To Enable Sound Despite Missing Data
        overwrite_calls(OptionButton_toggle, OptionButton_toggle_injection);
    }

    // Info Button
    if (feature_has("Add Reborn Info To Options", server_disabled)) {
        // Add Button
        overwrite_calls(OptionsScreen_init, OptionsScreen_init_injection);
        // Position Button
        overwrite_calls(OptionsScreen_setupPositions, OptionsScreen_setupPositions_injection);
        // Handle Click
        overwrite_calls(OptionsScreen_buttonClicked, OptionsScreen_buttonClicked_injection);
        // Cleanup
        overwrite_calls(OptionsScreen_removed, OptionsScreen_removed_injection);
    }
}
