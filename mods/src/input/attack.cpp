#include <cstdint>

#include <libreborn/patch.h>

#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include "input-internal.h"

// Add Attacking To MouseBuildInput
#define REMOVE_ATTACK_BAI 0xa
#define ATTACK_BAI 0x8
static bool MouseBuildInput_tickBuild_injection(MouseBuildInput_tickBuild_t original, MouseBuildInput *mouse_build_input, Player *local_player, uint32_t *build_action_intention_return) {
    // Call Original Method
    const bool ret = original(mouse_build_input, local_player, build_action_intention_return);
    // Convert Remove/Attack Into Attack If A Tile Is Not Selected
    if (ret && *build_action_intention_return == REMOVE_ATTACK_BAI) {
        const Minecraft *minecraft = ((LocalPlayer *) local_player)->minecraft;
        if (minecraft->hit_result.type != 0) {
            *build_action_intention_return = ATTACK_BAI;
        }
    }
    // Return
    return ret;
}
static void Minecraft_handleBuildAction_injection(Minecraft_handleBuildAction_t original, Minecraft *self, uint *bai) {
    if (*bai == ATTACK_BAI) {
        *bai = REMOVE_ATTACK_BAI;
    }
    original(self, bai);
}

// Init
void _init_attack() {
    // Allow Attacking Mobs
    if (feature_has("Fix Attacking", server_disabled)) {
        overwrite_calls(MouseBuildInput_tickBuild, MouseBuildInput_tickBuild_injection);
        overwrite_calls(Minecraft_handleBuildAction, Minecraft_handleBuildAction_injection);
    }
}
