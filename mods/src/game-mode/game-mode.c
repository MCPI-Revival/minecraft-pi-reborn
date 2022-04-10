#include "game-mode.h"
#include "../init/init.h"
#include "../feature/feature.h"

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

static int is_survival = -1;

// Patch Game Mode
static void set_is_survival(int new_is_survival) {
    if (is_survival != new_is_survival) {
        INFO("Setting Game Mode: %s", new_is_survival ? "Survival" : "Creative");

        // Correct Inventpry UI
        unsigned char inventory_patch[4] = {new_is_survival ? 0x00 : 0x01, 0x30, 0xa0, 0xe3}; // "mov r3, #0x0" or "mov r3, #0x1"
        patch((void *) 0x16efc, inventory_patch);

        // Use Correct Size For GameMode Object
        unsigned char size_patch[4] = {new_is_survival ? SURVIVAL_MODE_SIZE : CREATOR_MODE_SIZE, 0x00, 0xa0, 0xe3}; // "mov r0, #SURVIVAL_MODE_SIZE" or "mov r0, #CREATOR_MODE_SIZE"
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

// Disable CreatorMode-Specific API Features (Polling Block Hits) In SurvivalMode, This Is Preferable To Crashing
static unsigned char *Minecraft_getCreator_injection(unsigned char *minecraft) {
    if (is_survival) {
        // SurvivalMode, Return NULL
        return NULL;
    } else {
        // CreatorMode, Call Original Method
        return (*Minecraft_getCreator)(minecraft);
    }
}

// Init
void init_game_mode() {
    // Dynamic Game Mode Switching
    if (feature_has("Implement Game-Mode Switching", server_enabled)) {
        set_is_survival(1);
        overwrite_calls((void *) Minecraft_setIsCreativeMode, (void *) Minecraft_setIsCreativeMode_injection);

        // Replace CreatorLevel With ServerLevel (This Fixes Beds And Mob Spawning)
        overwrite_call((void *) 0x16f84, (void *) ServerLevel);

        // Allocate Correct Size For ServerLevel
        uint32_t level_size = SERVER_LEVEL_SIZE;
        patch_address((void *) 0x17004, (void *) level_size);

        // Disable CreatorMode-Specific API Features (Polling Block Hits) In SurvivalMode, This Is Preferable To Crashing
        overwrite_calls((void *) Minecraft_getCreator, (void *) Minecraft_getCreator_injection);

        // Init C++
        _init_game_mode_cpp();
    }

    // Allow Joining Survival Servers
    if (feature_has("Allow Joining Survival Servers", server_enabled)) {
        unsigned char server_patch[4] = {0x0f, 0x00, 0x00, 0xea}; // "b 0x6dcb4"
        patch((void *) 0x6dc70, server_patch);
    }
}
