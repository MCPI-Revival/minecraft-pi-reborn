#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "../init/init.h"
#include "../feature/feature.h"
#include "misc.h"

// Maximum Username Length
#define MAX_USERNAME_LENGTH 16

// Render Selected Item Text
static void Gui_renderChatMessages_injection(unsigned char *gui, int32_t param_1, uint32_t param_2, uint32_t param_3, unsigned char *font) {
    // Call Original Method
    (*Gui_renderChatMessages)(gui, param_1, param_2, param_3, font);
    // Calculate Selected Item Text Scale
    unsigned char *minecraft = *(unsigned char **) (gui + Gui_minecraft_property_offset);
    int32_t screen_width = *(int32_t *) (minecraft + Minecraft_screen_width_property_offset);
    float scale = ((float) screen_width) * *InvGuiScale;
    // Render Selected Item Text
    (*Gui_renderOnSelectItemNameText)(gui, (int32_t) scale, font, param_1 - 0x13);
}
// Reset Selected Item Text Timer On Slot Select
static uint32_t reset_selected_item_text_timer = 0;
static void Gui_tick_injection(unsigned char *gui) {
    // Call Original Method
    (*Gui_tick)(gui);
    // Handle Reset
    float *selected_item_text_timer = (float *) (gui + Gui_selected_item_text_timer_property_offset);
    if (reset_selected_item_text_timer) {
        // Reset
        *selected_item_text_timer = 0;
        reset_selected_item_text_timer = 0;
    }
}
// Trigger Reset Selected Item Text Timer On Slot Select
static void Inventory_selectSlot_injection(unsigned char *inventory, int32_t slot) {
    // Call Original Method
    (*Inventory_selectSlot)(inventory, slot);
    // Trigger Reset Selected Item Text Timer
    reset_selected_item_text_timer = 1;
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
    return (*RakNet_RakString)(rak_string, "%s", format);
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

// Init
void init_misc() {
    if (feature_has("Remove Invalid Item Background", 0)) {
        // Remove Invalid Item Background (A Red Background That Appears For Items That Are Not Included In The gui_blocks Atlas)
        unsigned char invalid_item_background_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x63c98, invalid_item_background_patch);
    }

    // Fix Selected Item Text
    if (feature_has("Render Selected Item Text", 0)) {
        overwrite_calls((void *) Gui_renderChatMessages, (void *) Gui_renderChatMessages_injection);
        overwrite_calls((void *) Gui_tick, (void *) Gui_tick_injection);
        overwrite_calls((void *) Inventory_selectSlot, (void *) Inventory_selectSlot_injection);
    }

    // Sanitize Username
    patch_address(LoginPacket_read_vtable_addr, (void *) LoginPacket_read_injection);

    // Fix RakNet::RakString Security Bug
    overwrite_calls((void *) RakNet_RakString, (void *) RakNet_RakString_injection);

    // Print Error Message If RakNet Startup Fails
    overwrite_call((void *) 0x73778, (void *) RakNetInstance_host_RakNet_RakPeer_Startup_injection);

    // Fix Bug Where RakNetInstance Starts Pinging Potential Servers Before The "Join Game" Screen Is Opened
    overwrite_calls((void *) RakNetInstance, (void *) RakNetInstance_injection);

    // Init C++
    _init_misc_cpp();
}
