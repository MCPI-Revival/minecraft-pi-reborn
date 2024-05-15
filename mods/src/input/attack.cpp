#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include "input-internal.h"
#include <mods/input/input.h>

// Store Left Click (0 = Not Pressed, 1 = Pressed)
static int is_left_click = 0;
void input_set_is_left_click(int val) {
    if ((is_left_click == 0 && val == 1) || (is_left_click != 0 && val == 0)) {
        is_left_click = val;
    }
}

// Add Attacking To MouseBuildInput
static int32_t MouseBuildInput_tickBuild_injection(MouseBuildInput_tickBuild_t original, MouseBuildInput *mouse_build_input, Player *local_player, uint32_t *build_action_intention_return) {
    // Call Original Method
    int32_t ret = original(mouse_build_input, local_player, build_action_intention_return);

    // Use Attack/Place BuildActionIntention If No Other Valid BuildActionIntention Was Selected And This Was Not A Repeated Left Click
    if (ret != 0 && is_left_click == 1 && *build_action_intention_return == 0xa) {
        // Get Target HitResult
        Minecraft *minecraft = ((LocalPlayer *) local_player)->minecraft;
        HitResult *hit_result = &minecraft->hit_result;
        int32_t hit_result_type = hit_result->type;
        // Check if The Target Is An Entity Using HitResult
        if (hit_result_type == 1) {
            // Change BuildActionIntention To Attack/Place Mode (Place Will Not Happen Because The HitResult Is An Entity)
            *build_action_intention_return = 0x8;
        }
    }

    // Return
    return ret;
}

// Fix Holding Attack
static bool last_player_attack_successful = false;
static bool Player_attack_Entity_hurt_injection(Entity *entity, Entity *attacker, int32_t damage) {
    // Call Original Method
    last_player_attack_successful = entity->hurt(attacker, damage);
    return last_player_attack_successful;
}
static ItemInstance *Player_attack_Inventory_getSelected_injection(Inventory *inventory) {
    // Check If Attack Was Successful
    if (!last_player_attack_successful) {
        return nullptr;
    }

    // Call Original Method
    return inventory->getSelected();
}

// Init
void _init_attack() {
    // Allow Attacking Mobs
    if (feature_has("Fix Attacking", server_disabled)) {
        overwrite_virtual_calls(MouseBuildInput_tickBuild, MouseBuildInput_tickBuild_injection);

        // Fix Holding Attack
        overwrite_call((void *) 0x8fc1c, (void *) Player_attack_Entity_hurt_injection);
        overwrite_call((void *) 0x8fc24, (void *) Player_attack_Inventory_getSelected_injection);
    }
}
