#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <GLES/gl.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <SDL/SDL.h>
#include <media-layer/core.h>

#include "../init/init.h"
#include "../feature/feature.h"
#include "misc.h"

// Maximum Username Length
#define MAX_USERNAME_LENGTH 16

// Additional GUI Rendering
static int hide_chat_messages = 0;
static int render_selected_item_text = 0;
static void Gui_renderChatMessages_injection(unsigned char *gui, int32_t y_offset, uint32_t max_messages, bool disable_fading, unsigned char *font) {
    // Call Original Method
    if (!hide_chat_messages) {
        (*Gui_renderChatMessages)(gui, y_offset, max_messages, disable_fading, font);
    }

    // Render Selected Item Text
    if (render_selected_item_text) {
        // Fix GL Mode
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Calculate Selected Item Text Scale
        unsigned char *minecraft = *(unsigned char **) (gui + Gui_minecraft_property_offset);
        int32_t screen_width = *(int32_t *) (minecraft + Minecraft_screen_width_property_offset);
        float scale = ((float) screen_width) * *InvGuiScale;
        // Render Selected Item Text
        (*Gui_renderOnSelectItemNameText)(gui, (int32_t) scale, font, y_offset - 0x13);
    }
}
// Reset Selected Item Text Timer On Slot Select
static uint32_t reset_selected_item_text_timer = 0;
static void Gui_tick_injection(unsigned char *gui) {
    // Call Original Method
    (*Gui_tick)(gui);

    // Handle Reset
    if (render_selected_item_text) {
        float *selected_item_text_timer = (float *) (gui + Gui_selected_item_text_timer_property_offset);
        if (reset_selected_item_text_timer) {
            // Reset
            *selected_item_text_timer = 0;
            reset_selected_item_text_timer = 0;
        }
    }
}
// Trigger Reset Selected Item Text Timer On Slot Select
static void Inventory_selectSlot_injection(unsigned char *inventory, int32_t slot) {
    // Call Original Method
    (*Inventory_selectSlot)(inventory, slot);

    // Trigger Reset Selected Item Text Timer
    if (render_selected_item_text) {
        reset_selected_item_text_timer = 1;
    }
}

// Sanitize Username
static void LoginPacket_read_injection(unsigned char *packet, unsigned char *bit_stream) {
    // Call Original Method
    (*LoginPacket_read)(packet, bit_stream);

    // Prepare
    unsigned char *rak_string = packet + LoginPacket_username_property_offset;
    // Get Original Username
    unsigned char *shared_string = *(unsigned char **) (rak_string + RakNet_RakString_sharedString_property_offset);
    char *c_str = *(char **) (shared_string + RakNet_RakString_SharedString_c_str_property_offset);
    // Sanitize
    char *new_username = strdup(c_str);
    ALLOC_CHECK(new_username);
    sanitize_string(&new_username, MAX_USERNAME_LENGTH, 0);
    // Set New Username
    (*RakNet_RakString_Assign)(rak_string, new_username);
    // Free
    free(new_username);
}

// Fix RakNet::RakString Security Bug
//
// RakNet::RakString's format constructor is often given unsanitized user input and is never used for formatting,
// this is a massive security risk, allowing clients to run arbitrary format specifiers, this disables the
// formatting functionality.
static unsigned char *RakNet_RakString_injection(unsigned char *rak_string, const char *format, ...) {
    // Call Original Method
    return (*RakNet_RakString)(rak_string, format);
}

// Print Error Message If RakNet Startup Fails
static char *RAKNET_ERROR_NAMES[] = {
    "Success",
    "Already Started",
    "Invalid Socket Descriptors",
    "Invalid Max Connections",
    "Socket Family Not Supported",
    "Part Already In Use",
    "Failed To Bind Port",
    "Failed Test Send",
    "Port Cannot Be 0",
    "Failed To Create Network Thread",
    "Couldn't Generate GUID",
    "Unknown"
};
#ifdef MCPI_SERVER_MODE
#define PRINT_RAKNET_STARTUP_FAILURE ERR
#else
#define PRINT_RAKNET_STARTUP_FAILURE WARN
#endif
static RakNet_StartupResult RakNetInstance_host_RakNet_RakPeer_Startup_injection(unsigned char *rak_peer, unsigned short maxConnections, unsigned char *socketDescriptors, uint32_t socketDescriptorCount, int32_t threadPriority) {
    // Call Original Method
    RakNet_StartupResult result = (*RakNet_RakPeer_Startup)(rak_peer, maxConnections, socketDescriptors, socketDescriptorCount, threadPriority);

    // Print Error
    if (result != RAKNET_STARTED) {
        PRINT_RAKNET_STARTUP_FAILURE("Failed To Start RakNet: %s", RAKNET_ERROR_NAMES[result]);
    }

    // Return
    return result;
}

// Fix Bug Where RakNetInstance Starts Pinging Potential Servers Before The "Join Game" Screen Is Opened
static unsigned char *RakNetInstance_injection(unsigned char *rak_net_instance) {
    // Call Original Method
    unsigned char *result = (*RakNetInstance)(rak_net_instance);
    // Fix
    *(unsigned char *) (rak_net_instance + RakNetInstance_pinging_for_hosts_property_offset) = 0;
    // Return
    return result;
}

// Close Current Screen On Death To Prevent Bugs
static void LocalPlayer_die_injection(unsigned char *entity, unsigned char *cause) {
    // Close Screen
    unsigned char *minecraft = *(unsigned char **) (entity + LocalPlayer_minecraft_property_offset);
    (*Minecraft_setScreen)(minecraft, NULL);

    // Call Original Method
    (*LocalPlayer_die)(entity, cause);
}

// Fix Furnace Not Checking Item Auxiliary When Inserting New Item
static int32_t FurnaceScreen_handleAddItem_injection(unsigned char *furnace_screen, int32_t slot, ItemInstance const *item) {
    // Get Existing Item
    unsigned char *tile_entity = *(unsigned char **) (furnace_screen + FurnaceScreen_tile_entity_property_offset);
    unsigned char *tile_entity_vtable = *(unsigned char **) tile_entity;
    FurnaceTileEntity_getItem_t FurnaceTileEntity_getItem = *(FurnaceTileEntity_getItem_t *) (tile_entity_vtable + FurnaceTileEntity_getItem_vtable_offset);
    ItemInstance *existing_item = (*FurnaceTileEntity_getItem)(tile_entity, slot);

    // Check Item
    int valid;
    if (item->id == existing_item->id && item->auxiliary == existing_item->auxiliary) {
        // Item Matches, Is Valid
        valid = 1;
    } else {
        // Item Doesn't Match, Check If Existing Item Is Empty
        if ((existing_item->id | existing_item->count | existing_item->auxiliary) == 0) {
            // Existing Item Is Empty, Is Valid
            valid = 1;
        } else {
            // Existing Item Isn't Empty, Isn't Valid
            valid = 0;
        }
    }

    // Call Original Method
    if (valid) {
        // Valid
        return (*FurnaceScreen_handleAddItem)(furnace_screen, slot, item);
    } else {
        // Invalid
        return 0;
    }
}

// Custom Cursor Rendering
//
// The default behavior for Touch GUI is to only render the cursor when the mouse is clicking, this fixes that.
// This also makes the cursor always render if the mouse is unlocked, instead of just when there is a Screen showing.
static void GameRenderer_render_injection(unsigned char *game_renderer, float param_1) {
    // Call Original Method
    (*GameRenderer_render)(game_renderer, param_1);

    // Check If Cursor Should Render
    if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF) {
        // Get X And Y
        float x = (*Mouse_getX)() * (*InvGuiScale);
        float y = (*Mouse_getY)() * (*InvGuiScale);
        // Render Cursor
        unsigned char *minecraft = *(unsigned char **) (game_renderer + GameRenderer_minecraft_property_offset);
        (*renderCursor)(x, y, minecraft);
    }
}

// Init
void init_misc() {
    // Remove Invalid Item Background (A Red Background That Appears For Items That Are Not Included In The gui_blocks Atlas)
    if (feature_has("Remove Invalid Item Background", server_disabled)) {
        unsigned char invalid_item_background_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x63c98, invalid_item_background_patch);
    }

    // Render Selected Item Text + Hide Chat Messages
    hide_chat_messages = feature_has("Hide Chat Messages", server_disabled);
    render_selected_item_text = feature_has("Render Selected Item Text", server_disabled);
    overwrite_calls((void *) Gui_renderChatMessages, (void *) Gui_renderChatMessages_injection);
    overwrite_calls((void *) Gui_tick, (void *) Gui_tick_injection);
    overwrite_calls((void *) Inventory_selectSlot, (void *) Inventory_selectSlot_injection);

    // Sanitize Username
    patch_address(LoginPacket_read_vtable_addr, (void *) LoginPacket_read_injection);

    // Fix RakNet::RakString Security Bug
    overwrite_calls((void *) RakNet_RakString, (void *) RakNet_RakString_injection);

    // Print Error Message If RakNet Startup Fails
    overwrite_call((void *) 0x73778, (void *) RakNetInstance_host_RakNet_RakPeer_Startup_injection);

    // Fix Bug Where RakNetInstance Starts Pinging Potential Servers Before The "Join Game" Screen Is Opened
    overwrite_calls((void *) RakNetInstance, (void *) RakNetInstance_injection);

    // Close Current Screen On Death To Prevent Bugs
    if (feature_has("Close Current Screen On Death", server_disabled)) {
        patch_address(LocalPlayer_die_vtable_addr, (void *) LocalPlayer_die_injection);
    }

    // Fix Furnace Not Checking Item Auxiliary When Inserting New Item
    if (feature_has("Fix Furnace Not Checking Item Auxiliary", server_disabled)) {
        overwrite_calls((void *) FurnaceScreen_handleAddItem, (void *) FurnaceScreen_handleAddItem_injection);
    }

    // Improved Cursor Rendering
    if (feature_has("Improved Cursor Rendering", server_disabled)) {
        // Disable Normal Cursor Rendering
        unsigned char disable_cursor_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x4a6c0, disable_cursor_patch);
        // Add Custom Cursor Rendering
        overwrite_calls((void *) GameRenderer_render, (void *) GameRenderer_render_injection);
    }

    // Disable V-Sync
    if (feature_has("Disable V-Sync", server_disabled)) {
        media_disable_vsync();
    }

    // Init C++ And Logging
    _init_misc_cpp();
    _init_misc_logging();
}
