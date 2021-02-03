#include <libreborn/libreborn.h>

#include "game_mode.h"
#include "../init/init.h"

#include <libreborn/minecraft.h>

static int is_survival = -1;

// Patch Game Mode
static void set_is_survival(int new_is_survival) {
    if (is_survival != new_is_survival) {
        INFO("Setting Game Mode: %s", new_is_survival ? "Survival" : "Creative");

        // Correct Inventpry UI
        unsigned char inventory_patch[4] = {new_is_survival ? 0x00 : 0x01, 0x30, 0xa0, 0xe3};
        patch((void *) 0x16efc, inventory_patch);

        // Use Correct Size For GameMode Object
        unsigned char size_patch[4] = {new_is_survival ? 0x24 : 0x18, 0x00, 0xa0, 0xe3};
        patch((void *) 0x16ee4, size_patch);

        // Replace Default CreatorMode Constructor With CreatorMode Or SurvivalMode Constructor
        overwrite_call((void *) 0x16ef4, new_is_survival ? SurvivalMode : CreatorMode);

        is_survival = new_is_survival;
    }
}

// Handle Gamemode Switching
static void Minecraft_setIsCreativeMode_injection(unsigned char *this, int32_t new_game_mode) {
    set_is_survival(!new_game_mode);

    // Call Original Method
    (*Minecraft_setIsCreativeMode)(this, new_game_mode);
}

void init_game_mode() {
    // Dynamic Game Mode Switching
    set_is_survival(1);
    overwrite_calls((void *) Minecraft_setIsCreativeMode, Minecraft_setIsCreativeMode_injection);

    // Replace CreatorLevel With ServerLevel (This Fixes Beds And Mob Spawning)
    unsigned char level_patch[4] = {0x68, 0x7e, 0x01, 0xeb};
    patch((void *) 0x16f84, level_patch);

    // Allocate Correct Size For ServerLevel
    unsigned char level_size_patch[4] = {0x94, 0x0b, 0x00, 0x00};
    patch((void *) 0x17004, level_size_patch);

    // Allow Connecting To Survival Servers
    unsigned char server_patch[4] = {0x0f, 0x00, 0x00, 0xea};
    patch((void *) 0x6dc70, server_patch);

    // Init C++
    init_game_mode_cpp();
}