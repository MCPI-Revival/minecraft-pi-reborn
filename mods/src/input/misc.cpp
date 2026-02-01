#include <libreborn/patch.h>

#include <symbols/Minecraft.h>
#include <symbols/Screen.h>
#include <symbols/OptionsScreen.h>
#include <symbols/InBedScreen.h>
#include <symbols/Touch_IngameBlockSelectionScreen.h>
#include <symbols/LocalPlayer.h>
#include <symbols/Tile.h>
#include <symbols/ScrollingPane.h>
#include <symbols/Mouse.h>
#include <symbols/Common.h>

#include <SDL/SDL.h>

#include "internal.h"

#include <mods/input/input.h>
#include <mods/feature/feature.h>
#include <mods/misc/misc.h>

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

// Block Item Dropping When Mouse Is Locked
static void Gui_tickItemDrop_injection(Gui_tickItemDrop_t original, Gui *self) {
    const Minecraft *minecraft = self->minecraft;
    const bool is_in_game = minecraft->screen == nullptr || minecraft->screen->vtable == (Screen::VTable *) Touch_IngameBlockSelectionScreen::VTable::base;
    if (media_SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF && is_in_game) {
        // Call Original Method
        return original(self);
    }
}

// Block UI Interaction When Mouse Is Locked
static void Gui_handleClick_injection(Gui_handleClick_t original, Gui *gui, const int32_t param_2, const int32_t param_3, const int32_t param_4) {
    if (media_SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF) {
        // Call Original Method
        original(gui, param_2, param_3, param_4);
    }
}

// Block Tile Interaction When Sneaking
static bool GameMode_useItemOn_Tile_use_injection(Tile *tile, Level *level, int x, int y, int z, Player *player) {
    if (player->isSneaking()) {
        return false;
    }
    // Call Original Method
    return tile->use(level, x, y, z, player);
}

// Inventory Scrolling
static constexpr float scroll_scale = 32; // Amount Of Scaled-Pixels To Move Per Scroll Event
static int scroll = 0;
void input_set_scroll(const int direction) {
    scroll = direction;
}
static void ScrollingPane_render_injection(ScrollingPane_render_t original, ScrollingPane *self, const int mouse_x, const int mouse_y, const float alpha) {
    // Handle Scroll
    if (scroll != 0) {
        const float scale = self->scale;
        const float x = float(Mouse::getX()) * scale;
        const float y = float(Mouse::getY()) * scale;
        if (self->area.isInside(x, y)) {
            // Prepare
            const int time = Common::getTimeMs();
            constexpr float touch_x = -999;
            float touch_y = touch_x;
            // Cancel Old Event
            self->touchesEnded(touch_x, touch_y, time);
            // Trigger Drag Tracking
            self->touchesBegan(touch_x, touch_y, time);
            touch_y *= 2;
            self->touchesMoved(touch_x, touch_y, time);
            // Actually Scroll
            const float diff = scroll_scale * float(scroll) * scale;
            touch_y += diff;
            self->touchesMoved(touch_x, touch_y, time);
            self->touchesEnded(touch_x, touch_y, time);
            // Prevent Ludicrous Velocity
            self->snapContentOffsetToBounds(false);
            self->deceleration_velocity = {
                .x = 0,
                .y = 0,
                .z = 0
            };
        }
    }

    // Call Original Method
    original(self, mouse_x, mouse_y, alpha);
}
static void reset_scroll(MCPI_UNUSED Minecraft *minecraft) {
    // Reset Scroll After Frame
    scroll = 0;
}

// Init
void _init_misc() {
    // Proper Back Button Handling
    if (feature_has("Fix Escape Key Handling", server_disabled)) {
        misc_run_on_key_press([](Minecraft *mc, const int key) {
            if (key == MC_KEY_ESCAPE) {
                _handle_back(mc);
                return true;
            } else {
                return false;
            }
        });
        // Fix OptionsScreen Ignoring The Back Button
        patch_vtable(OptionsScreen_handleBackEvent, OptionsScreen_handleBackEvent_injection);
        // Fix "Sleeping Beauty" Bug (https://discord.com/channels/740287937727561779/761048906242981948/1164426402318270474)
        patch_vtable(InBedScreen_handleBackEvent, InBedScreen_handleBackEvent_injection);
    }

    // Fix UI When Mouse Is Locked
    if (feature_has("Stop Locked Mouse From Interacting With HUD", server_disabled)) {
        // Disable Opening Inventory Using The Cursor When Cursor Is Hidden
        overwrite_calls(Gui_handleClick, Gui_handleClick_injection);
        // Disable Item Dropping Using The Cursor When Cursor Is Hidden
        overwrite_calls(Gui_tickItemDrop, Gui_tickItemDrop_injection);
    }

    // Sneaking
    if (feature_has("Block Tile Interaction When Sneaking", server_is_not_vanilla_compatible)) {
        // GameMode::useItemOn
        overwrite_call((void *) 0x1a870, Tile_use, GameMode_useItemOn_Tile_use_injection);
        // ServerSideNetworkHandler::handle_UseItemPacket
        overwrite_call((void *) 0x748ec, Tile_use, GameMode_useItemOn_Tile_use_injection);
    }

    // Inventory Scrolling
    if (feature_has("Implement Inventory Mouse Scrolling", server_disabled)) {
        overwrite_calls(ScrollingPane_render, ScrollingPane_render_injection);
        misc_run_on_update(reset_scroll);
    }
}