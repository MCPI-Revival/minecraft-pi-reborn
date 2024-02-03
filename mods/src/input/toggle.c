#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "input-internal.h"
#include <mods/input/input.h>
#include <mods/feature/feature.h>
#include <mods/misc/misc.h>

// Handle Toggle Options
static bool _handle_toggle_options(Minecraft *minecraft, int key) {
    Options *options = &minecraft->options;
    if (key == 0x70) {
        // Toggle Hide GUI
        options->hide_gui = options->hide_gui ^ 1;
        return 1;
    } else if (key == 0x74) {
        // Toggle Third Person
        options->third_person = (options->third_person + 1) % 3;
        return 1;
    } else {
        return 0;
    }
}
static void _fix_third_person(Minecraft *minecraft) {
    // Fix Broken Value From Third-Person OptionsButton Toggle
    // (Because Front-Facing Code Repurposes A Boolean As A Ternary)
    Options *options = &minecraft->options;
    if (options->third_person == 3) {
        options->third_person = 0;
    }
}

// Font-Facing View
static void invert_rotation(Entity *entity) {
    if (entity != NULL) {
        entity->yaw = 180.f + entity->yaw;
        entity->old_yaw = 180.f + entity->old_yaw;
        entity->pitch = -entity->pitch;
        entity->old_pitch = -entity->old_pitch;
    }
}
static void revert_rotation(Entity *entity) {
    if (entity != NULL) {
        entity->yaw = -180.f + entity->yaw;
        entity->old_yaw = -180.f + entity->old_yaw;
        entity->pitch = -entity->pitch;
        entity->old_pitch = -entity->old_pitch;
    }
}
static int is_front_facing = 0;
static LocalPlayer *stored_player = NULL;
static void GameRenderer_setupCamera_injection(GameRenderer *game_renderer, float param_1, int param_2) {
    // Get Objects
    Minecraft *minecraft = game_renderer->minecraft;
    stored_player = minecraft->player;

    // Check If In Third-Person
    Options *options = &minecraft->options;
    is_front_facing = (options->third_person == 2);

    // Invert Rotation
    if (is_front_facing) {
        invert_rotation((Entity *) stored_player);
    }

    // Call Original Method
    GameRenderer_setupCamera(game_renderer, param_1, param_2);

    // Revert
    if (is_front_facing) {
        revert_rotation((Entity *) stored_player);
    }
}
static void ParticleEngine_render_injection(ParticleEngine *particle_engine, Entity *entity, float param_2) {
    // Invert Rotation
    if (is_front_facing && (Entity *) stored_player == entity) {
        invert_rotation((Entity *) stored_player);
    }

    // Call Original Method
    ParticleEngine_render(particle_engine, entity, param_2);

    // Revert
    if (is_front_facing && (Entity *) stored_player == entity) {
        revert_rotation((Entity *) stored_player);
    }
}

// Init
void _init_toggle() {
    if (feature_has("Bind Common Toggleable Options To Function Keys", server_disabled)) {
        misc_run_on_game_key_press(_handle_toggle_options);
        misc_run_on_update(_fix_third_person);

        // Font-Facing View
        overwrite_calls((void *) GameRenderer_setupCamera, (void *) GameRenderer_setupCamera_injection);
        overwrite_calls((void *) ParticleEngine_render, (void *) ParticleEngine_render_injection);
    }
}
