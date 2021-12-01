#include <string.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>

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

static int fancy_graphics;
static int peaceful_mode;
static int anaglyph;
static int smooth_lighting;
static int render_distance;
static int server_visible;
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
    *(int32_t *) (options + Options_game_difficulty_property_offset) = peaceful_mode ? 0 : 2;
    // 3D Anaglyph
    *(options + Options_3d_anaglyph_property_offset) = anaglyph;
    // Smooth Lighting
    *(options + Options_ambient_occlusion_property_offset) = smooth_lighting;
    // Render Distance
    *(int32_t *) (options + Options_render_distance_property_offset) = render_distance;
    // Server Visible
    *(options + Options_server_visible_property_offset) = server_visible;
}

// Init
void init_options() {
    // Force Mob Spawning
    if (feature_has("Force Mob Spawning", server_auto)) {
        overwrite((void *) LevelData_getSpawnMobs, (void *) LevelData_getSpawnMobs_injection);
    }

    // Enable Fancy Graphics
    fancy_graphics = feature_has("Fancy Graphics", server_disabled);
    // Peaceful Mode
    peaceful_mode = feature_has("Peaceful Mode", server_auto);
    // 3D Anaglyph
    anaglyph = feature_has("3D Anaglyph", server_disabled);
    // Render Distance
    render_distance = get_render_distance();
    DEBUG("Setting Render Distance: %i", render_distance);
    // Server Visible
    server_visible = !feature_has("Disable Hosting LAN Worlds", server_disabled);

    // Set Options
    overwrite_calls((void *) Minecraft_init, (void *) Minecraft_init_injection);

    // Change Username
    const char *username = get_username();
    DEBUG("Setting Username: %s", username);
    if (strcmp(*default_username, "StevePi") != 0) {
        ERR("Default Username Is Invalid");
    }
    safe_username = to_cp437(username);
    patch_address((void *) default_username, (void *) safe_username);
    unsigned char username_length_patch[4] = {(unsigned char) strlen(safe_username), 0x20, 0xa0, 0xe3}; // "mov r2, #USERNAME_LENGTH"
    patch((void *) 0x1ba2c, username_length_patch);

    // Disable Autojump By Default
    if (feature_has("Disable Autojump By Default", server_disabled)) {
        unsigned char autojump_patch[4] = {0x00, 0x00, 0xa0, 0xe3}; // "mov r0, #0x0"
        patch((void *) 0x5b148, autojump_patch);
    }
    // Display Nametags By Default
    if (feature_has("Display Nametags By Default", server_disabled)) {
        // r6 = 0x1
        // r12 = 0x0
        unsigned char display_nametags_patch[4] = {0x1d, 0x60, 0xc4, 0xe5}; // "strb r6, [r4, #0x1d]"
        patch((void *) 0xf2d44, display_nametags_patch);
    }

    // Enable Smooth Lighting
    smooth_lighting = feature_has("Smooth Lighting", server_disabled);
    if (smooth_lighting) {
        unsigned char smooth_lighting_patch[4] = {0x01, 0x00, 0x53, 0xe3}; // "cmp r3, #0x1"
        patch((void *) 0x73b74, smooth_lighting_patch);
    }
}
