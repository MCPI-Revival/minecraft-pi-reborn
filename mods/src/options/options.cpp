#include <cstring>
#include <string>
#include <vector>
#include <sstream>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include <mods/home/home.h>

// Force Mob Spawning
static bool LevelData_getSpawnMobs_injection(__attribute__((unused)) unsigned char *level_data) {
    return true;
}

// Get Custom Render Distance
static int get_render_distance() {
    const char *distance_str = getenv("MCPI_RENDER_DISTANCE");
    if (distance_str == nullptr) {
        distance_str = "Short";
    }
    if (strcmp("Far", distance_str) == 0) {
        return 0;
    } else if (strcmp("Normal", distance_str) == 0) {
        return 1;
    } else if (strcmp("Short", distance_str) == 0) {
        return 2;
    } else if (strcmp("Tiny", distance_str) == 0) {
        return 3;
    } else {
        ERR("Invalid Render Distance: %s", distance_str);
    }
}

// Get Custom Username
static const char *get_username() {
    const char *username = getenv("MCPI_USERNAME");
    if (username == nullptr) {
        username = "StevePi";
    }
    return username;
}
static char *safe_username = nullptr;
__attribute__((destructor)) static void _free_safe_username() {
    free(safe_username);
}

static int render_distance;
// Configure Options
static Options *stored_options = nullptr;
static void Options_initDefaultValue_injection(Options_initDefaultValue_t original, Options *options) {
    // Call Original Method
    original(options);

    // Default Graphics Settings
#ifndef MCPI_SERVER_MODE
    options->fancy_graphics = true;
    options->ambient_occlusion = true;
#endif

    // Store
    stored_options = options;
}
static void Minecraft_init_injection(Minecraft_init_t original, Minecraft *minecraft) {
    // Call Original Method
    original(minecraft);

    Options *options = &minecraft->options;
    // Enable Crosshair In Touch GUI
    options->split_controls = true;
    // Render Distance
    options->render_distance = render_distance;
}

// Smooth Lighting
static bool TileRenderer_tesselateBlockInWorld_injection(TileRenderer_tesselateBlockInWorld_t original, TileRenderer *tile_renderer, Tile *tile, int32_t x, int32_t y, int32_t z) {
    // Set Variable
    Minecraft_useAmbientOcclusion = stored_options->ambient_occlusion;

    // Call Original Method
    return original(tile_renderer, tile, x, y, z);
}

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

// Actually Save options.txt
// Hook Last Options::addOptionToSaveOutput Call
static void Options_save_Options_addOptionToSaveOutput_injection(Options *options, std::vector<std::string> *data, std::string option, int32_t value) {
    // Call Original Method
    Options_addOptionToSaveOutput(options, data, option, value);

    // Save Fancy Graphics
    Options_addOptionToSaveOutput(options, data, "gfx_fancygraphics", options->fancy_graphics);

    // Save 3D Anaglyph
    Options_addOptionToSaveOutput(options, data, "gfx_anaglyph", options->anaglyph_3d);

    // Save File
    OptionsFile *options_file = &options->options_file;
    OptionsFile_save(options_file, data);
}

// MCPI's OptionsFile::getOptionStrings is broken, this is the version in v0.7.0
static std::vector<std::string> OptionsFile_getOptionStrings_injection(OptionsFile *options_file) {
    // Get options.txt Path
    std::string path = options_file->options_txt_path;
    // Parse
    std::vector<std::string> ret;
    FILE *stream = fopen(path.c_str(), "r");
    char line[128];
    if (stream != nullptr) {
        while (fgets(line, 0x80, stream) != nullptr) {
            size_t sVar1 = strlen(line);
            if (2 < sVar1) {
                std::stringstream string_stream(line);
                while (true) {
                    std::string data;
                    std::getline(string_stream, data, ':');
                    int iVar2 = data.find_last_not_of(" \n\r\t");
                    data.erase(iVar2 + 1);
                    if (data.length() == 0) {
                        break;
                    }
                    ret.push_back(data);
                }
            }
        }
        fclose(stream);
    }
    return ret;
}

// Get New options.txt Path
#ifndef MCPI_SERVER_MODE
static char *get_new_options_txt_path() {
    static char *path = nullptr;
    // Path
    if (path == nullptr) {
        safe_asprintf(&path, "%s/options.txt", home_get());
    }
    // Return
    return path;
}
// Free
__attribute__((destructor)) static void _free_new_options_txt_path() {
    free(get_new_options_txt_path());
}
#else
static char *get_new_options_txt_path() {
    // Block options.txt On Servers
    return (char *) "/dev/null";
}
#endif

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
void init_options() {
    // Force Mob Spawning
    if (feature_has("Force Mob Spawning", server_auto)) {
        overwrite((void *) LevelData_getSpawnMobs, (void *) LevelData_getSpawnMobs_injection);
    }

    // Render Distance
    render_distance = get_render_distance();
    DEBUG("Setting Render Distance: %i", render_distance);

    // Set Options
    overwrite_calls(Options_initDefaultValue, Options_initDefaultValue_injection);
    overwrite_virtual_calls(Minecraft_init, Minecraft_init_injection);

    // Change Username
    const char *username = get_username();
    DEBUG("Setting Username: %s", username);
    if (strcmp(Strings_default_username, "StevePi") != 0) {
        ERR("Default Username Is Invalid");
    }
    safe_username = to_cp437(username);
    patch_address((void *) Strings_default_username_pointer, (void *) safe_username);

    // Disable Autojump By Default
    if (feature_has("Disable Autojump By Default", server_disabled)) {
        unsigned char autojump_patch[4] = {0x00, 0x30, 0xa0, 0xe3}; // "mov r3, #0x0"
        patch((void *) 0x44b90, autojump_patch);
    }
    // Display Nametags By Default
    if (feature_has("Display Nametags By Default", server_disabled)) {
        // r6 = 0x1
        // r5 = 0x0
        unsigned char display_nametags_patch[4] = {0x1d, 0x60, 0xc0, 0xe5}; // "strb r6, [r0, #0x1d]"
        patch((void *) 0xa6628, display_nametags_patch);
    }

    // Smooth Lighting
    overwrite_calls(TileRenderer_tesselateBlockInWorld, TileRenderer_tesselateBlockInWorld_injection);

    // NOP
    unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"

    // Fix Options Screen
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

    // Actually Save options.txt
    overwrite_call((void *) 0x197fc, (void *) Options_save_Options_addOptionToSaveOutput_injection);
    // Fix options.txt Path
    patch_address((void *) Strings_options_txt_path_pointer, (void *) get_new_options_txt_path());
    // When Loading, options.txt Should Be Opened In Read Mode
    patch_address((void *) Strings_options_txt_fopen_mode_when_loading_pointer, (void *) "r");
    // Fix OptionsFile::getOptionStrings
    overwrite(OptionsFile_getOptionStrings, OptionsFile_getOptionStrings_injection);

    // Sensitivity Loading/Saving Is Broken, Disable It
    patch((void *) 0x1931c, nop_patch);
    patch((void *) 0x1973c, nop_patch);

    // Unsplit Touch Controls Breaks Things, Never Load/Save It
    unsigned char cmp_r0_r0_patch[4] = {0x00, 0x00, 0x50, 0xe1}; // "cmp r0, r0"
    patch((void *) 0x19378, cmp_r0_r0_patch);
    patch((void *) 0x197cc, nop_patch);

    // Custom Username Is Loaded Manually, Disable Loading From options.txt
    patch((void *) 0x192ac, nop_patch);

    // Replace "feedback_vibration" Loading/Saving With "gfx_ao"
    {
        // Replace String
        patch_address((void *) Strings_feedback_vibration_options_txt_name_pointer, (void *) "gfx_ao");
        // Loading
        unsigned char offset = (unsigned char) offsetof(Options, ambient_occlusion);
        unsigned char gfx_ao_loading_patch[4] = {offset, 0x10, 0x84, 0xe2}; // "add r1, r4, #OFFSET"
        patch((void *) 0x193b8, gfx_ao_loading_patch);
        // Saving
        unsigned char gfx_ao_saving_patch[4] = {offset, 0x30, 0xd4, 0xe5}; // "ldrb r3, [r4, #OFFSET]"
        patch((void *) 0x197f8, gfx_ao_saving_patch);
    }

    // Replace "gfx_lowquality" Loading With "gfx_anaglyph"
    {
        // Replace String
        patch_address((void *) Strings_gfx_lowquality_options_txt_name_pointer, (void *) "gfx_anaglyph");
        // Loading
        unsigned char offset = (unsigned char) offsetof(Options, anaglyph_3d);
        unsigned char gfx_anaglyph_loading_patch[4] = {offset, 0x10, 0x84, 0xe2}; // "add r1, r4, #OFFSET"
        patch((void *) 0x19400, gfx_anaglyph_loading_patch);
        // Disable Loading Side Effects
        patch((void *) 0x19414, nop_patch);
        patch((void *) 0x1941c, nop_patch);
    }
}
