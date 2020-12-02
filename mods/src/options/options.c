#include <string.h>

#include <libcore/libcore.h>

#include "../feature/feature.h"
#include "../init/init.h"

#include "../minecraft.h"

static int mob_spawning = 0;
// Override Mob Spawning
static uint32_t LevelData_getSpawnMobs_injection(__attribute__((unused)) unsigned char *level_data) {
    return mob_spawning;
}

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
// Configure Options
static void Minecraft_init_injection(unsigned char *this) {
    // Call Original Method
    (*Minecraft_init)(this);

    unsigned char *options = this + 0x3c;
    // Enable Fancy Graphics
    *(options + 0x17) = fancy_graphics;
    // Enable Crosshair In Touch GUI
    *(options + 0x105) = 1;
    // Peaceful Mode
    *(int32_t *) (options + 0xe8) = peaceful_mode ? 0 : 2;
    // 3D Anaglyph
    *(options + 0x15) = anaglyph;
    // Smooth Lighting
    *(options + 0x18) = smooth_lighting;
}

// Enable Touch GUI
static int32_t Minecraft_isTouchscreen_injection(__attribute__((unused)) unsigned char *minecraft) {
    return 1;
}

void init_options() {
    int is_server = feature_get_mode() == 2;

    int touch_gui = feature_has("Touch GUI");
    if (touch_gui) {
        // Main UI
        overwrite((void *) Minecraft_isTouchscreen, Minecraft_isTouchscreen_injection);
        // Force Correct Toolbar Size
        unsigned char toolbar_patch[4] = {0x01, 0x00, 0x50, 0xe3};
        patch((void *) 0x257b0, toolbar_patch);
    }

    mob_spawning = feature_has("Mob Spawning");
    // Set Mob Spawning
    overwrite((void *) LevelData_getSpawnMobs, LevelData_getSpawnMobs_injection);

    // Enable Fancy Graphics
    fancy_graphics = feature_has("Fancy Graphics");
    // Peaceful Mode
    peaceful_mode = feature_has("Peaceful Mode");
    // 3D Anaglyph
    anaglyph = feature_has("3D Anaglyph");

    // Set Options
    overwrite_calls((void *) Minecraft_init, Minecraft_init_injection);

    // Change Username
    const char *username = get_username();
    if (!is_server) {
        INFO("Setting Username: %s", username);
    }
    if (strcmp(*default_username, "StevePi") != 0) {
        ERR("%s", "Default Username Is Invalid");
    }
    patch_address((void *) default_username, (void *) username);

    if (feature_has("Disable Autojump By Default")) {
        // Disable Autojump By Default
        unsigned char autojump_patch[4] = {0x00, 0x30, 0xa0, 0xe3};
        patch((void *) 0x44b90, autojump_patch);
    }

    // Show Block Outlines
    int block_outlines = feature_has("Show Block Outlines");
    unsigned char outline_patch[4] = {block_outlines ? !touch_gui : touch_gui, 0x00, 0x50, 0xe3};
    patch((void *) 0x4a210, outline_patch);

    smooth_lighting = feature_has("Smooth Lighting");
    if (smooth_lighting) {
        // Enable Smooth Lighting
        unsigned char smooth_lighting_patch[4] = {0x01, 0x00, 0x53, 0xe3};
        patch((void *) 0x59ea4, smooth_lighting_patch);
    }
}