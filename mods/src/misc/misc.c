#include <string.h>

#include <libreborn/libreborn.h>
#include <libreborn/minecraft.h>

#include "../feature/feature.h"
#include "misc.h"
#include "../init/init.h"

// Minecraft Pi User Data Root
#define NEW_PATH "/.minecraft-pi/"

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

void init_misc() {
    // Store Data In ~/.minecraft-pi Instead Of ~/.minecraft
    patch_address((void *) default_path, (void *) NEW_PATH);

    if (feature_has("Remove Invalid Item Background")) {
        // Remove Invalid Item Background (A Red Background That Appears For Items That Are Not Included In The gui_blocks Atlas)
        unsigned char invalid_item_background_patch[4] = {0x00, 0xf0, 0x20, 0xe3};
        patch((void *) 0x63c98, invalid_item_background_patch);
    }

    // Fix Selected Item Text
    overwrite_calls((void *) Gui_renderChatMessages, (void *) Gui_renderChatMessages_injection);
    overwrite_calls((void *) Gui_tick, (void *) Gui_tick_injection);
    overwrite_calls((void *) Inventory_selectSlot, (void *) Inventory_selectSlot_injection);

    // Sanitize Username
    patch_address(LoginPacket_read_vtable_addr, (void *) LoginPacket_read_injection);

    // Init C++
    init_misc_cpp();
}