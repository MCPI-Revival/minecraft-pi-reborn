#include <cstring>
#include <string>
#include <vector>
#include <sstream>

#include <libreborn/patch.h>
#include <libreborn/config.h>
#include <libreborn/env.h>
#include <libreborn/string.h>
#include <libreborn/util.h>

#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>

#include "options-internal.h"

// Force Mob Spawning
static bool LevelData_getSpawnMobs_injection(__attribute__((unused)) LevelData_getSpawnMobs_t original, __attribute__((unused)) LevelData *level_data) {
    return true;
}

// Get Custom Render Distance
static int get_render_distance() {
    const char *distance_str = getenv(MCPI_RENDER_DISTANCE_ENV);
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
    const char *username = getenv(MCPI_USERNAME_ENV);
    if (username == nullptr) {
        username = "StevePi";
    }
    return username;
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

    // Save Fancy Graphics
    options->addOptionToSaveOutput(data, "gfx_fancygraphics", options->fancy_graphics);

    // Save 3D Anaglyph
    options->addOptionToSaveOutput(data, "gfx_anaglyph", options->anaglyph_3d);

    // Save File
    OptionsFile *options_file = &options->options_file;
    options_file->save(data);
}

// MCPI's OptionsFile::getOptionStrings is broken, this is the version in v0.7.0
static std::vector<std::string> OptionsFile_getOptionStrings_injection(__attribute__((unused)) OptionsFile_getOptionStrings_t original, OptionsFile *options_file) {
    // Get options.txt Path
    const std::string path = options_file->options_txt_path;
    // Parse
    std::vector<std::string> ret;
    FILE *stream = fopen(path.c_str(), "r");
    if (stream != nullptr) {
        char line[128];
        while (fgets(line, 0x80, stream) != nullptr) {
            const size_t sVar1 = strlen(line);
            if (2 < sVar1) {
                std::stringstream string_stream(line);
                while (true) {
                    std::string data;
                    std::getline(string_stream, data, ':');
                    const int iVar2 = data.find_last_not_of(" \n\r\t");
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
    overwrite_calls(Options_initDefaultValue, Options_initDefaultValue_injection);
    overwrite_calls(Minecraft_init, Minecraft_init_injection);

    // Change Username
    const char *username = get_username();
    DEBUG("Setting Username: %s", username);
    if (strcmp(Strings::default_username, "StevePi") != 0) {
        ERR("Default Username Is Invalid");
    }
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

    // NOP
    unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"

    // Actually Save options.txt
    overwrite_call((void *) 0x197fc, (void *) Options_save_Options_addOptionToSaveOutput_injection);
    // Fix options.txt Path
    patch_address((void *) &Strings::options_txt_path, (void *) get_new_options_txt_path());
    // When Loading, options.txt Should Be Opened In Read Mode
    patch_address((void *) &Strings::options_txt_fopen_mode_when_loading, (void *) "r");
    // Fix OptionsFile::getOptionStrings
    overwrite_calls(OptionsFile_getOptionStrings, OptionsFile_getOptionStrings_injection);

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
        patch_address((void *) &Strings::feedback_vibration_options_txt_name, (void *) "gfx_ao");
        // Loading
        constexpr unsigned char offset = (unsigned char) offsetof(Options, ambient_occlusion);
        unsigned char gfx_ao_loading_patch[4] = {offset, 0x10, 0x84, 0xe2}; // "add r1, r4, #OFFSET"
        patch((void *) 0x193b8, gfx_ao_loading_patch);
        // Saving
        unsigned char gfx_ao_saving_patch[4] = {offset, 0x30, 0xd4, 0xe5}; // "ldrb r3, [r4, #OFFSET]"
        patch((void *) 0x197f8, gfx_ao_saving_patch);
    }

    // Replace "gfx_lowquality" Loading With "gfx_anaglyph"
    {
        // Replace String
        patch_address((void *) &Strings::gfx_lowquality_options_txt_name, (void *) "gfx_anaglyph");
        // Loading
        constexpr unsigned char offset = (unsigned char) offsetof(Options, anaglyph_3d);
        unsigned char gfx_anaglyph_loading_patch[4] = {offset, 0x10, 0x84, 0xe2}; // "add r1, r4, #OFFSET"
        patch((void *) 0x19400, gfx_anaglyph_loading_patch);
        // Disable Loading Side Effects
        patch((void *) 0x19414, nop_patch);
        patch((void *) 0x1941c, nop_patch);
    }

    // UI
    _init_options_ui();
}
