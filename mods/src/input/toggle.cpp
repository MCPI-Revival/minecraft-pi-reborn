#include <libreborn/patch.h>

#include <symbols/minecraft.h>

#include "internal.h"
#include <mods/input/input.h>
#include <mods/feature/feature.h>
#include <mods/misc/misc.h>

// Handle Toggle Options
static bool _handle_toggle_options(Minecraft *minecraft, const int key) {
    Options *options = &minecraft->options;
    if (key == MC_KEY_F1) {
        // Toggle Hide GUI
        options->hide_gui = options->hide_gui ^ 1;
        return true;
    } else if (key == MC_KEY_F5) {
        // Toggle Third Person
        options->third_person = (options->third_person + 1) % 3;
        return true;
    } else {
        return false;
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
    if (entity != nullptr) {
        entity->yaw = 180.f + entity->yaw;
        entity->old_yaw = 180.f + entity->old_yaw;
        entity->pitch = -entity->pitch;
        entity->old_pitch = -entity->old_pitch;
    }
}
static void revert_rotation(Entity *entity) {
    if (entity != nullptr) {
        entity->yaw = -180.f + entity->yaw;
        entity->old_yaw = -180.f + entity->old_yaw;
        entity->pitch = -entity->pitch;
        entity->old_pitch = -entity->old_pitch;
    }
}
static bool is_front_facing = false;
static Entity *stored_player = nullptr;
static void GameRenderer_setupCamera_injection(GameRenderer_setupCamera_t original, GameRenderer *game_renderer, const float param_1, const int param_2) {
    // Get Objects
    const Minecraft *minecraft = game_renderer->minecraft;
    stored_player = (Entity *) minecraft->camera;

    // Check If In Third-Person
    const Options *options = &minecraft->options;
    is_front_facing = (options->third_person == 2);

    // Invert Rotation
    if (is_front_facing) {
        invert_rotation(stored_player);
    }

    // Call Original Method
    original(game_renderer, param_1, param_2);

    // Revert
    if (is_front_facing) {
        revert_rotation(stored_player);
    }
}
static void ParticleEngine_render_injection(ParticleEngine_render_t original, ParticleEngine *particle_engine, Entity *entity, const float param_2) {
    // Invert Rotation
    if (is_front_facing && stored_player == entity) {
        invert_rotation(stored_player);
    }

    // Call Original Method
    original(particle_engine, entity, param_2);

    // Revert
    if (is_front_facing && stored_player == entity) {
        revert_rotation(stored_player);
    }
}

// Hide Crosshair
static void Gui_renderProgressIndicator_GuiComponent_blit_injection(GuiComponent *self, int x1, int y1, int x2, int y2, int w1, int h1, int w2, int h2) {
    if (((Gui *) self)->minecraft->options.third_person == 0) {
        self->blit(x1, y1, x2, y2, w1, h1, w2, h2);
    }
}

// Init
void _init_toggle() {
    if (feature_has("Bind Common Toggleable Options To Function Keys", server_disabled)) {
        misc_run_on_game_key_press(_handle_toggle_options);
        misc_run_on_update(_fix_third_person);

        // Font-Facing View
        overwrite_calls(GameRenderer_setupCamera, GameRenderer_setupCamera_injection);
        overwrite_calls(ParticleEngine_render, ParticleEngine_render_injection);
    }
    if (feature_has("Hide Crosshair In Third-Person", server_disabled)) {
        overwrite_call((void *) 0x261b8, GuiComponent_blit, Gui_renderProgressIndicator_GuiComponent_blit_injection);
    }
}
