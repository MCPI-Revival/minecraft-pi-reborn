#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <SDL/SDL.h>

#include "input-internal.h"
#include <mods/input/input.h>
#include <mods/feature/feature.h>
#include <mods/creative/creative.h>
#include <mods/misc/misc.h>

// Enable Miscellaneous Input Fixes
static int enable_misc = 0;

// Handle Back Button Presses
static void _handle_back(Minecraft *minecraft) {
    // If Minecraft's Level property is initialized, but Minecraft's Player property is nullptr, then Minecraft::handleBack may crash.
    if (minecraft->level != nullptr && minecraft->player == nullptr) {
        // Unable to safely run Minecraft::handleBack, deferring until safe.
        return;
    }
    // Send Event
    minecraft->handleBack(false);
}

// Fix OptionsScreen Ignoring The Back Button
static bool OptionsScreen_handleBackEvent_injection(OptionsScreen *screen, const bool do_nothing) {
    if (!do_nothing) {
        Minecraft *minecraft = screen->minecraft;
        minecraft->setScreen(nullptr);
    }
    return true;
}

// Fix "Sleeping Beauty" Bug
static bool InBedScreen_handleBackEvent_injection(InBedScreen *screen, const bool do_nothing) {
    if (!do_nothing) {
        // Close Screen
        Minecraft *minecraft = screen->minecraft;
        minecraft->setScreen(nullptr);
        // Stop Sleeping
        LocalPlayer *player = minecraft->player;
        if (player != nullptr) {
            player->stopSleepInBed(true, true, true);
        }
    }
    return true;
}

// Block UI Interaction When Mouse Is Locked
static bool Gui_tickItemDrop_Minecraft_isCreativeMode_call_injection(Minecraft *minecraft) {
    const bool is_in_game = minecraft->screen == nullptr || minecraft->screen->vtable == (Screen_vtable *) Touch_IngameBlockSelectionScreen_vtable::base;
    if (!enable_misc || (media_SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF && is_in_game)) {
        // Call Original Method
        return creative_is_restricted() && minecraft->isCreativeMode();
    } else {
        // Disable Item Drop Ticking
        return true;
    }
}

// Block UI Interaction When Mouse Is Locked
static void Gui_handleClick_injection(Gui_handleClick_t original, Gui *gui, const int32_t param_2, const int32_t param_3, const int32_t param_4) {
    if (media_SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF) {
        // Call Original Method
        original(gui, param_2, param_3, param_4);
    }
}

// Init
void _init_misc() {
    enable_misc = feature_has("Miscellaneous Input Fixes", server_disabled);
    if (enable_misc) {
        // Fix OptionsScreen Ignoring The Back Button
        patch_vtable(OptionsScreen_handleBackEvent, OptionsScreen_handleBackEvent_injection);
        // Fix "Sleeping Beauty" Bug
        patch_vtable(InBedScreen_handleBackEvent, InBedScreen_handleBackEvent_injection);
        // Disable Opening Inventory Using The Cursor When Cursor Is Hidden
        overwrite_calls(Gui_handleClick, Gui_handleClick_injection);
        // Proper Back Button Handling
        misc_run_on_key_press([](Minecraft *mc, const int key) {
            if (key == MC_KEY_ESCAPE) {
                _handle_back(mc);
                return true;
            } else {
                return false;
            }
        });
    }
    // Disable Item Dropping Using The Cursor When Cursor Is Hidden
    overwrite_call((void *) 0x27800, (void *) Gui_tickItemDrop_Minecraft_isCreativeMode_call_injection);
}
