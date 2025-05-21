#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>

#include <libreborn/patch.h>
#include <libreborn/env/env.h>
#include <libreborn/util/string.h>
#include <libreborn/util/util.h>
#include <libreborn/config.h>

#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include <mods/options/options.h>

#include "internal.h"

// Force Mob Spawning
static bool LevelData_getSpawnMobs_injection(MCPI_UNUSED LevelData_getSpawnMobs_t original, MCPI_UNUSED LevelData *level_data) {
    return true;
}

// Get Custom Render Distance
static int get_render_distance() {
    const char *distance_str = require_env(MCPI_RENDER_DISTANCE_ENV);
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

static int render_distance;
// Configure Options
Options *stored_options = nullptr;
static void Options_initDefaultValue_injection(Options_initDefaultValue_t original, Options *options) {
    // Call Original Method
    original(options);

    // Default Graphics Settings
    options->fancy_graphics = true;
    options->ambient_occlusion = true;
    // Mute If Sound Data Is Missing
    if (info_sound_data_state != info_sound_data_loaded) {
        options->sound = 0;
    }
}
static void Minecraft_init_injection(Minecraft_init_t original, Minecraft *minecraft) {
    // Call Original Method
    original(minecraft);

    Options *options = &minecraft->options;
    // Enable Crosshair In Touch GUI
    options->split_controls = true;
    // Render Distance
    options->render_distance = render_distance;

    // Store
    stored_options = options;
}

// Smooth Lighting
static bool TileRenderer_tesselateBlockInWorld_injection(TileRenderer_tesselateBlockInWorld_t original, TileRenderer *tile_renderer, Tile *tile, const int32_t x, const int32_t y, const int32_t z) {
    // Set Variable
    Minecraft::useAmbientOcclusion = stored_options->ambient_occlusion;

    // Call Original Method
    return original(tile_renderer, tile, x, y, z);
}

// Actually Save options.txt
// Hook Last Options::addOptionToSaveOutput Call
static void Options_save_Options_addOptionToSaveOutput_injection(Options *options, std::vector<std::string> *data, std::string option, int32_t value) {
    // Call Original Method
    options->addOptionToSaveOutput(data, option, value);

    // Save Smooth Lighting
    options->addOptionToSaveOutput(data, "gfx_ao", options->ambient_occlusion);
    // Save Fancy Graphics
    options->addOptionToSaveOutput(data, "gfx_fancygraphics", options->fancy_graphics);
    // Save 3D Anaglyph
    options->addOptionToSaveOutput(data, "gfx_anaglyph", options->anaglyph_3d);

    // Save File
    OptionsFile *options_file = &options->options_file;
    options_file->save(data);
}

// MCPI's OptionsFile::getOptionStrings is broken, this is modified from the version in v0.7.0
static std::vector<std::string> OptionsFile_getOptionStrings_v2(OptionsFile *options_file) {
    // Get options.txt Path
    const std::string path = options_file->options_txt_path;
    // Parse
    std::vector<std::string> ret;
    std::ifstream stream(path);
    if (stream) {
        std::string line;
        while (std::getline(stream, line)) {
            if (!line.empty()) {
                std::stringstream string_stream(line);
                std::string part;
                while (std::getline(string_stream, part, ':')) {
                    ret.push_back(part);
                }
            }
        }
        stream.close();
    }
    return ret;
}

// Replacement Of Options::update
static void Options_update_injection(MCPI_UNUSED Options_update_t original, Options *self) {
    const std::vector<std::string> strings = OptionsFile_getOptionStrings_v2(&self->options_file);
    for (std::vector<std::string>::size_type i = 0; i < strings.size(); i++) {
        // Read
        const std::string key = strings[i++];
        if (i == strings.size()) {
            // Missing Value
            break;
        }
        const std::string value = strings[i];
        if (key == "mp_server_visible_default") {
            Options::readBool(value, self->server_visible);
        } else if (key == "game_difficulty") {
            int &difficulty = self->game_difficulty;
            Options::readInt(value, difficulty);
            constexpr int normal_difficulty = 2;
            if (difficulty != 0 && difficulty != normal_difficulty) {
                difficulty = normal_difficulty;
            }
        } else if (key == "ctrl_invertmouse") {
            Options::readBool(value, self->invert_mouse);
        } else if (key == "ctrl_islefthanded") {
            Options::readBool(value, self->lefty);
        } else if (key == "gfx_ao") {
            Options::readBool(value, self->ambient_occlusion);
        } else if (key == "gfx_fancygraphics") {
            Options::readBool(value, self->fancy_graphics);
        } else if (key == "gfx_anaglyph") {
            Options::readBool(value, self->anaglyph_3d);
        } else if (key == "ctrl_usetouchscreen" || key == "feedback_vibration") {
            // Skip
        } else {
            WARN("Unknown Option: %s", key.c_str());
        }
    }
}

// Get New options.txt Path
static const char *get_new_options_txt_path() {
    static std::string path = "";
    // Path
    if (path.empty()) {
        path = !reborn_is_server() ? (std::string(home_get()) + "/options.txt") : "/dev/null";
    }
    // Return
    return path.c_str();
}

// Init
void init_options() {
    // Force Mob Spawning
    if (feature_has("Force Mob Spawning", server_auto)) {
        overwrite_calls(LevelData_getSpawnMobs, LevelData_getSpawnMobs_injection);
    }

    // Render Distance
    render_distance = get_render_distance();
    DEBUG("Setting Render Distance: %i", render_distance);

    // Set Options
    if (feature_has("Update Default Options", server_disabled)) {
        overwrite_calls(Options_initDefaultValue, Options_initDefaultValue_injection);
    }
    overwrite_calls(Minecraft_init, Minecraft_init_injection);

    // Change Username
    const char *username = require_env(MCPI_USERNAME_ENV);
    DEBUG("Setting Username: %s", username);
    static std::string safe_username = to_cp437(username);
    patch_address((void *) &Strings::default_username, (void *) safe_username.c_str());

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

    // options.txt
    if (feature_has("Fix options.txt Loading/Saving", server_enabled)) {
        // Actually Save options.txt
        overwrite_call((void *) 0x197fc, Options_addOptionToSaveOutput, Options_save_Options_addOptionToSaveOutput_injection);
        // Fix options.txt Path
        patch_address((void *) &Strings::options_txt_path, (void *) get_new_options_txt_path());
        // When Loading, options.txt Should Be Opened In Read Mode
        patch_address((void *) &Strings::options_txt_fopen_mode_when_loading, (void *) "r");
        // Fix Loading
        overwrite_calls(Options_update, Options_update_injection);

        // Disable Saving Some Settings
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x1973c, nop_patch); // "ctrl_sensitivity"
        patch((void *) 0x197cc, nop_patch); // "ctrl_usetouchjoypad"
    }

    // UI
    _init_options_ui();
}
