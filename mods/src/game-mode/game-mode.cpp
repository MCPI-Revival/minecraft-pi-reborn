#include <libreborn/patch.h>

#include <symbols/Minecraft.h>
#include <symbols/GameMode.h>
#include <symbols/CreatorMode.h>
#include <symbols/SurvivalMode.h>
#include <symbols/LocalPlayer.h>
#include <symbols/ServerLevel.h>
#include <symbols/CreatorLevel.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>

#include "internal.h"

// Handle Game-Mode Switching
static void Minecraft_setIsCreativeMode_injection(MCPI_UNUSED Minecraft_setIsCreativeMode_t original, Minecraft *self, const bool new_is_creative) {
    // Log
    DEBUG("Setting Game Mode: %s", new_is_creative ? "Creative" : "Survival");
    self->is_creative_mode = new_is_creative;
    // Destroy Old Game-Mode
    GameMode *&game_mode = self->game_mode;
    if (game_mode) {
        game_mode->destructor_deleting();
        game_mode = nullptr;
    }
    // Construct New Game-Mode
    if (new_is_creative) {
        CreatorMode *creator_mode = CreatorMode::allocate();
        creator_mode->constructor(self);
        game_mode = (GameMode *) creator_mode;
    } else {
        SurvivalMode *survival_mode = SurvivalMode::allocate();
        survival_mode->constructor(self);
        game_mode = (GameMode *) survival_mode;
    }
    // Setup Player
    LocalPlayer *player = self->player;
    if (player) {
        game_mode->initAbilities(player->abilities);
    }
}

// Disable CreatorMode-Specific API Features (Polling Block Hits) In SurvivalMode, This Is Preferable To Crashing
static ICreator *Minecraft_getCreator_injection(Minecraft_getCreator_t original, Minecraft *minecraft) {
    if (!minecraft->is_creative_mode) {
        // SurvivalMode, Return nullptr
        return nullptr;
    } else {
        // CreatorMode, Call Original Method
        return original(minecraft);
    }
}

// Replace CreatorLevel With ServerLevel
static CreatorLevel *Minecraft_selectLevel_CreatorMode_injection(CreatorLevel *self, LevelStorage *storage, const std::string &name, const LevelSettings &settings, int param_4, Dimension *dimension) {
    ((ServerLevel *) self)->constructor(storage, name, settings, param_4, dimension);
    return self;
}

// Init
void init_game_mode() {
    // Dynamic Game Mode Switching
    if (feature_has("Implement Game-Mode Switching", server_enabled)) {
        overwrite_calls(Minecraft_setIsCreativeMode, Minecraft_setIsCreativeMode_injection);

        // Replace CreatorLevel With ServerLevel (This Fixes Beds And Mob Spawning)
        overwrite_call((void *) 0x16f84, CreatorLevel_constructor, Minecraft_selectLevel_CreatorMode_injection);
        // Allocate The Correct Size For ServerLevel
        constexpr uint level_size = sizeof(ServerLevel);
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
