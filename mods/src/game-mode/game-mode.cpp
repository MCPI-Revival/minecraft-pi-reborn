#include "game-mode-internal.h"
#include <mods/init/init.h>
#include <mods/feature/feature.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

static int is_survival = -1;

// Patch Game Mode
static void set_is_survival(bool new_is_survival) {
    if (is_survival != new_is_survival) {
        DEBUG("Setting Game Mode: %s", new_is_survival ? "Survival" : "Creative");

        // Correct Inventpry UI
        unsigned char inventory_patch[4] = {(unsigned char) (new_is_survival ? 0x00 : 0x01), 0x30, 0xa0, 0xe3}; // "mov r3, #0x0" or "mov r3, #0x1"
        patch((void *) 0x16efc, inventory_patch);

        // Use Correct Size For GameMode Object
        unsigned char size_patch[4] = {(unsigned char) (new_is_survival ? SURVIVAL_MODE_SIZE : CREATOR_MODE_SIZE), 0x00, 0xa0, 0xe3}; // "mov r0, #SURVIVAL_MODE_SIZE" or "mov r0, #CREATOR_MODE_SIZE"
        patch((void *) 0x16ee4, size_patch);

        // Replace Default CreatorMode Constructor With CreatorMode Or SurvivalMode Constructor
        overwrite_call((void *) 0x16ef4, new_is_survival ? (void *) SurvivalMode_constructor : (void *) CreatorMode_constructor);

        is_survival = new_is_survival;
    }
}

// Handle Gamemode Switching
static void Minecraft_setIsCreativeMode_injection(Minecraft_setIsCreativeMode_t original, Minecraft *self, int32_t new_game_mode) {
    set_is_survival(!new_game_mode);

    // Call Original Method
    original(self, new_game_mode);
}

// Disable CreatorMode-Specific API Features (Polling Block Hits) In SurvivalMode, This Is Preferable To Crashing
static unsigned char *Minecraft_getCreator_injection(Minecraft_getCreator_t original, Minecraft *minecraft) {
    if (is_survival) {
        // SurvivalMode, Return nullptr
        return nullptr;
    } else {
        // CreatorMode, Call Original Method
        return original(minecraft);
    }
}

// Init
void init_game_mode() {
    // Dynamic Game Mode Switching
    if (feature_has("Implement Game-Mode Switching", server_enabled)) {
        set_is_survival(true);
        overwrite_calls(Minecraft_setIsCreativeMode, Minecraft_setIsCreativeMode_injection);

        // Replace CreatorLevel With ServerLevel (This Fixes Beds And Mob Spawning)
        overwrite_call((void *) 0x16f84, (void *) ServerLevel_constructor);

        // Allocate Correct Size For ServerLevel
        uint32_t level_size = SERVER_LEVEL_SIZE;
        patch_address((void *) 0x17004, (void *) level_size);

        // Disable CreatorMode-Specific API Features (Polling Block Hits) In SurvivalMode, This Is Preferable To Crashing
        overwrite_calls(Minecraft_getCreator, Minecraft_getCreator_injection);
    }

    // Create World Dialog
    if (feature_has("Implement Create World Dialog", server_disabled)) {
        // Init UI
        _init_game_mode_ui();
    }

    // Allow Joining Survival Mode Servers
    if (feature_has("Allow Joining Survival Mode Servers", server_enabled)) {
        unsigned char server_patch[4] = {0x0f, 0x00, 0x00, 0xea}; // "b 0x6dcb4"
        patch((void *) 0x6dc70, server_patch);
    }
}
