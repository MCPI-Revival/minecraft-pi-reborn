#include <string.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include "options-internal.h"

// Force Mob Spawning
static bool LevelData_getSpawnMobs_injection(__attribute__((unused)) unsigned char *level_data) {
    return 1;
}

// Get Custom Render Distance
static int get_render_distance() {
    char *distance_str = getenv("MCPI_RENDER_DISTANCE");
    if (distance_str == NULL) {
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
static char *get_username() {
    char *username = getenv("MCPI_USERNAME");
    if (username == NULL) {
        username = "StevePi";
    }
    return username;
}
static char *safe_username = NULL;
__attribute__((destructor)) static void _free_safe_username() {
    free(safe_username);
}

static int anaglyph;
static int render_distance;
// Configure Options
unsigned char *stored_options = NULL;
static void Options_initDefaultValue_injection(unsigned char *options) {
    // Call Original Method
    (*Options_initDefaultValue)(options);

    // Default Graphics Settings
#ifndef MCPI_SERVER_MODE
    *(options + Options_fancy_graphics_property_offset) = 1;
    *(options + Options_ambient_occlusion_property_offset) = 1;
#endif

    // Store
    stored_options = options;
}
static void Minecraft_init_injection(unsigned char *minecraft) {
    // Call Original Method
    (*Minecraft_init)(minecraft);

    unsigned char *options = minecraft + Minecraft_options_property_offset;
    // Enable Crosshair In Touch GUI
    *(options + Options_split_controls_property_offset) = 1;
    // 3D Anaglyph
    *(options + Options_3d_anaglyph_property_offset) = anaglyph;
    // Render Distance
    *(int32_t *) (options + Options_render_distance_property_offset) = render_distance;
}

// Smooth Lighting
static void TileRenderer_tesselateBlockInWorld_injection(unsigned char *tile_renderer, unsigned char *tile, int32_t x, int32_t y, int32_t z) {
    // Set Variable
    *Minecraft_useAmbientOcclusion = *(stored_options + Options_ambient_occlusion_property_offset);

    // Call Original Method
    (*TileRenderer_tesselateBlockInWorld)(tile_renderer, tile, x, y, z);
}

// Init
void init_options() {
    // Force Mob Spawning
    if (feature_has("Force Mob Spawning", server_auto)) {
        overwrite((void *) LevelData_getSpawnMobs, (void *) LevelData_getSpawnMobs_injection);
    }

    // 3D Anaglyph
    anaglyph = feature_has("3D Anaglyph", server_disabled);
    // Render Distance
    render_distance = get_render_distance();
    DEBUG("Setting Render Distance: %i", render_distance);

    // Set Options
    overwrite_calls((void *) Options_initDefaultValue, (void *) Options_initDefaultValue_injection);
    overwrite_calls((void *) Minecraft_init, (void *) Minecraft_init_injection);

    // Change Username
    const char *username = get_username();
    DEBUG("Setting Username: %s", username);
    if (strcmp(*default_username, "StevePi") != 0) {
        ERR("Default Username Is Invalid");
    }
    safe_username = to_cp437(username);
    patch_address((void *) default_username, (void *) safe_username);

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
    overwrite_calls((void *) TileRenderer_tesselateBlockInWorld, (void *) TileRenderer_tesselateBlockInWorld_injection);

    // Init C++
    _init_options_cpp();
}
