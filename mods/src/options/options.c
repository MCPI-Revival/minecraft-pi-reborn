#include <string.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "../feature/feature.h"
#include "../init/init.h"

// Force Mob Spawning
static bool LevelData_getSpawnMobs_injection(__attribute__((unused)) unsigned char *level_data) {
    return 1;
}

#ifndef MCPI_SERVER_MODE
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
#endif // #ifndef MCPI_SERVER_MODE

// Get Custom Username
static char *get_username() {
    char *username = getenv("MCPI_USERNAME");
    if (username == NULL) {
        username = "StevePi";
    }
    return username;
}

static int fancy_graphics;
static int peaceful_mode;
static int anaglyph;
static int smooth_lighting;
static int render_distance;
// Configure Options
static void Minecraft_init_injection(unsigned char *this) {
    // Call Original Method
    (*Minecraft_init)(this);

    unsigned char *options = this + Minecraft_options_property_offset;
    // Enable Fancy Graphics
    *(options + Options_fancy_graphics_property_offset) = fancy_graphics;
    // Enable Crosshair In Touch GUI
    *(options + Options_split_controls_property_offset) = 1;
    // Peaceful Mode
    *(int32_t *) (options + Options_peaceful_mode_property_offset) = peaceful_mode ? 0 : 2;
    // 3D Anaglyph
    *(options + Options_3d_anaglyph_property_offset) = anaglyph;
    // Smooth Lighting
    *(options + Options_ambient_occlusion_property_offset) = smooth_lighting;
    // Render Distance
    *(int32_t *) (options + Options_render_distance_property_offset) = render_distance;
}

// Init
void init_options() {
    // Force Mob Spawning
    if (feature_has("Force Mob Spawning", -1)) {
        overwrite((void *) LevelData_getSpawnMobs, (void *) LevelData_getSpawnMobs_injection);
    }

    // Enable Fancy Graphics
    fancy_graphics = feature_has("Fancy Graphics", 0);
    // Peaceful Mode
    peaceful_mode = feature_has("Peaceful Mode", -1);
    // 3D Anaglyph
    anaglyph = feature_has("3D Anaglyph", 0);
    // Render Distance
#ifndef MCPI_SERVER_MODE
    render_distance = get_render_distance();
    INFO("Setting Render Distance: %i", render_distance);
#else // #ifndef MCPI_SERVER_MODE
    render_distance = 3;
#endif // #ifndef MCPI_SERVER_MODE

    // Set Options
    overwrite_calls((void *) Minecraft_init, (void *) Minecraft_init_injection);

    // Change Username
    const char *username = get_username();
#ifndef MCPI_SERVER_MODE
    INFO("Setting Username: %s", username);
#endif // #ifndef MCPI_SERVER_MODE
    if (strcmp(*default_username, "StevePi") != 0) {
        ERR("%s", "Default Username Is Invalid");
    }
    patch_address((void *) default_username, (void *) username);

    // Disable Autojump By Default
    if (feature_has("Disable Autojump By Default", 0)) {
        unsigned char autojump_patch[4] = {0x00, 0x30, 0xa0, 0xe3}; // "mov r3, #0x0"
        patch((void *) 0x44b90, autojump_patch);
    }
    // Display Nametags By Default
    if (feature_has("Display Nametags By Default", 0)) {
        // r6 = 0x1
        // r5 = 0x0
        unsigned char display_nametags_patch[4] = {0x1d, 0x60, 0xc0, 0xe5}; // "strb r6, [r0, #0x1d]"
        patch((void *) 0xa6628, display_nametags_patch);
    }

    // Enable Smooth Lighting
    smooth_lighting = feature_has("Smooth Lighting", 0);
    if (smooth_lighting) {
        unsigned char smooth_lighting_patch[4] = {0x01, 0x00, 0x53, 0xe3}; // "cmp r3, #0x1"
        patch((void *) 0x59ea4, smooth_lighting_patch);
    }
}
