#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <GLES/gl.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <SDL/SDL.h>
#include <media-layer/core.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include "misc-internal.h"
#include <mods/misc/misc.h>

// Classic HUD
#define DEFAULT_HUD_PADDING 2
#define NEW_HUD_PADDING 1
#define HUD_ELEMENT_WIDTH 82
#define HUD_ELEMENT_HEIGHT 9
#define TOOLBAR_HEIGHT 22
#define SLOT_WIDTH 20
#define DEFAULT_BUBBLES_PADDING 1
#define NUMBER_OF_SLOTS 9
static int use_classic_hud = 0;
static void Gui_renderHearts_GuiComponent_blit_hearts_injection(unsigned char *component, int32_t x_dest, int32_t y_dest, int32_t x_src, int32_t y_src, int32_t width_dest, int32_t height_dest, int32_t width_src, int32_t height_src) {
    unsigned char *minecraft = *(unsigned char **) (component + Gui_minecraft_property_offset);
    x_dest -= DEFAULT_HUD_PADDING;
    float width = ((float) *(int32_t *) (minecraft + Minecraft_screen_width_property_offset)) * *InvGuiScale;
    float height = ((float) *(int32_t *) (minecraft + Minecraft_screen_height_property_offset)) * *InvGuiScale;
    x_dest += (width - (NUMBER_OF_SLOTS * SLOT_WIDTH)) / 2;
    y_dest -= DEFAULT_HUD_PADDING;
    y_dest += height - HUD_ELEMENT_HEIGHT - TOOLBAR_HEIGHT - NEW_HUD_PADDING;
    // Call Original Method
    (*GuiComponent_blit)(component, x_dest, y_dest, x_src, y_src, width_dest, height_dest, width_src, height_src);
}
static void Gui_renderHearts_GuiComponent_blit_armor_injection(unsigned char *component, int32_t x_dest, int32_t y_dest, int32_t x_src, int32_t y_src, int32_t width_dest, int32_t height_dest, int32_t width_src, int32_t height_src) {
    unsigned char *minecraft = *(unsigned char **) (component + Gui_minecraft_property_offset);
    x_dest -= DEFAULT_HUD_PADDING + HUD_ELEMENT_WIDTH;
    float width = ((float) *(int32_t *) (minecraft + Minecraft_screen_width_property_offset)) * *InvGuiScale;
    float height = ((float) *(int32_t *) (minecraft + Minecraft_screen_height_property_offset)) * *InvGuiScale;
    x_dest += width - ((width - (NUMBER_OF_SLOTS * SLOT_WIDTH)) / 2) - HUD_ELEMENT_WIDTH;
    y_dest -= DEFAULT_HUD_PADDING;
    y_dest += height - HUD_ELEMENT_HEIGHT - TOOLBAR_HEIGHT - NEW_HUD_PADDING;
    // Call Original Method
    (*GuiComponent_blit)(component, x_dest, y_dest, x_src, y_src, width_dest, height_dest, width_src, height_src);
}
static void Gui_renderBubbles_GuiComponent_blit_injection(unsigned char *component, int32_t x_dest, int32_t y_dest, int32_t x_src, int32_t y_src, int32_t width_dest, int32_t height_dest, int32_t width_src, int32_t height_src) {
    unsigned char *minecraft = *(unsigned char **) (component + Gui_minecraft_property_offset);
    x_dest -= DEFAULT_HUD_PADDING;
    float width = ((float) *(int32_t *) (minecraft + Minecraft_screen_width_property_offset)) * *InvGuiScale;
    float height = ((float) *(int32_t *) (minecraft + Minecraft_screen_height_property_offset)) * *InvGuiScale;
    x_dest += (width - (NUMBER_OF_SLOTS * SLOT_WIDTH)) / 2;
    y_dest -= DEFAULT_HUD_PADDING + DEFAULT_BUBBLES_PADDING + HUD_ELEMENT_HEIGHT;
    y_dest += height - HUD_ELEMENT_HEIGHT - TOOLBAR_HEIGHT - HUD_ELEMENT_HEIGHT - NEW_HUD_PADDING;
    // Call Original Method
    (*GuiComponent_blit)(component, x_dest, y_dest, x_src, y_src, width_dest, height_dest, width_src, height_src);
}

// Additional GUI Rendering
static int hide_chat_messages = 0;
static int render_selected_item_text = 0;
static void Gui_renderChatMessages_injection(unsigned char *gui, int32_t y_offset, uint32_t max_messages, bool disable_fading, unsigned char *font) {
    // Handle Classic HUD
    if (use_classic_hud) {
        unsigned char *minecraft = *(unsigned char **) (gui + Gui_minecraft_property_offset);
        if (!(*Minecraft_isCreativeMode)(minecraft)) {
            y_offset -= (HUD_ELEMENT_HEIGHT * 2) + NEW_HUD_PADDING;
        }
    }

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

// Translucent Toolbar
static void Gui_renderToolBar_injection(unsigned char *gui, float param_1, int32_t param_2, int32_t param_3) {
    // Call Original Method
    int was_blend_enabled = glIsEnabled(GL_BLEND);
    if (!was_blend_enabled) {
        glEnable(GL_BLEND);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    (*Gui_renderToolBar)(gui, param_1, param_2, param_3);
    if (!was_blend_enabled) {
        glDisable(GL_BLEND);
    }
}
static void Gui_renderToolBar_glColor4f_injection(GLfloat red, GLfloat green, GLfloat blue, __attribute__((unused)) GLfloat alpha) {
    // Fix Alpha
    glColor4f(red, green, blue, 1.0f);
}

// Fix Screen Rendering When GUI is Hidden
static void Screen_render_injection(unsigned char *screen, int32_t param_1, int32_t param_2, float param_3) {
    // Fix
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Call Original Method
    (*Screen_render)(screen, param_1, param_2, param_3);
}

// Sanitize Username
#define MAX_USERNAME_LENGTH 16
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
        // Fix GL Mode
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Get X And Y
        float x = (*Mouse_getX)() * (*InvGuiScale);
        float y = (*Mouse_getY)() * (*InvGuiScale);
        // Render Cursor
        unsigned char *minecraft = *(unsigned char **) (game_renderer + GameRenderer_minecraft_property_offset);
        (*renderCursor)(x, y, minecraft);
    }
}

// Get Real Selected Slot
int32_t misc_get_real_selected_slot(unsigned char *player) {
    // Get Selected Slot
    unsigned char *inventory = *(unsigned char **) (player + Player_inventory_property_offset);
    int32_t selected_slot = *(int32_t *) (inventory + Inventory_selectedSlot_property_offset);

    // Linked Slots
    int32_t linked_slots_length = *(int32_t *) (inventory + FillingContainer_linked_slots_length_property_offset);
    if (selected_slot < linked_slots_length) {
        int32_t *linked_slots = *(int32_t **) (inventory + FillingContainer_linked_slots_property_offset);
        selected_slot = linked_slots[selected_slot];
    }

    // Return
    return selected_slot;
}

// Properly Generate Buffers
static void anGenBuffers_injection(int32_t count, uint32_t *buffers) {
    glGenBuffers(count, buffers);
}

// Fix Graphics Bug When Switching To First-Person While Sneaking
static void HumanoidMobRenderer_render_injection(unsigned char *model_renderer, unsigned char *entity, float param_2, float param_3, float param_4, float param_5, float param_6) {
    (*HumanoidMobRenderer_render)(model_renderer, entity, param_2, param_3, param_4, param_5, param_6);
    unsigned char *model = *(unsigned char **) (model_renderer + HumanoidMobRenderer_model_property_offset);
    *(bool *) (model + HumanoidModel_is_sneaking_property_offset) = 0;
}

// Custom API Port
HOOK(bind, int, (int sockfd, const struct sockaddr *addr, socklen_t addrlen)) {
    const struct sockaddr *new_addr = addr;
    struct sockaddr_in in_addr;
    if (addr->sa_family == AF_INET) {
        in_addr = *(const struct sockaddr_in *) new_addr;
        if (in_addr.sin_port == ntohs(4711)) {
            const char *new_port_str = getenv("MCPI_API_PORT");
            long int new_port;
            if (new_port_str != NULL && (new_port = strtol(new_port_str, NULL, 0)) != 0L) {
                in_addr.sin_port = htons(new_port);
            }
        }
        new_addr = (const struct sockaddr *) &in_addr;
    }
    ensure_bind();
    return (*real_bind)(sockfd, new_addr, addrlen);
}

// Init
static void nop() {
}
void init_misc() {
    // Remove Invalid Item Background (A Red Background That Appears For Items That Are Not Included In The gui_blocks Atlas)
    if (feature_has("Remove Invalid Item Background", server_disabled)) {
        unsigned char invalid_item_background_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x63c98, invalid_item_background_patch);
    }

    // Classic HUD
    if (feature_has("Classic HUD", server_disabled)) {
        use_classic_hud = 1;
        overwrite_call((void *) 0x266f8, (void *) Gui_renderHearts_GuiComponent_blit_hearts_injection);
        overwrite_call((void *) 0x26758, (void *) Gui_renderHearts_GuiComponent_blit_hearts_injection);
        overwrite_call((void *) 0x267c8, (void *) Gui_renderHearts_GuiComponent_blit_hearts_injection);
        overwrite_call((void *) 0x2656c, (void *) Gui_renderHearts_GuiComponent_blit_armor_injection);
        overwrite_call((void *) 0x268c4, (void *) Gui_renderBubbles_GuiComponent_blit_injection);
    }

    // Render Selected Item Text + Hide Chat Messages
    hide_chat_messages = feature_has("Hide Chat Messages", server_disabled);
    render_selected_item_text = feature_has("Render Selected Item Text", server_disabled);
    overwrite_calls((void *) Gui_renderChatMessages, (void *) Gui_renderChatMessages_injection);
    overwrite_calls((void *) Gui_tick, (void *) Gui_tick_injection);
    overwrite_calls((void *) Inventory_selectSlot, (void *) Inventory_selectSlot_injection);

    // Translucent Toolbar
    if (feature_has("Translucent Toolbar", server_disabled)) {
        overwrite_calls((void *) Gui_renderToolBar, (void *) Gui_renderToolBar_injection);
        overwrite_call((void *) 0x26c5c, (void *) Gui_renderToolBar_glColor4f_injection);
    }

    // Fix Screen Rendering When GUI is Hidden
    overwrite_calls((void *) Screen_render, (void *) Screen_render_injection);

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
    if (feature_has("Disable V-Sync", server_enabled)) {
        media_disable_vsync();
    }

    // Force EGL
    if (feature_has("Force EGL", server_disabled)) {
        media_force_egl();
    }

    // Remove Forced GUI Lag
    if (feature_has("Remove Forced GUI Lag (Can Break Joining Servers)", server_enabled)) {
        overwrite_calls((void *) sleepMs, (void *) nop);
    }

    // Properly Generate Buffers
    overwrite((void *) anGenBuffers, (void *) anGenBuffers_injection);

    // Fix Graphics Bug When Switching To First-Person While Sneaking
    patch_address(PlayerRenderer_render_vtable_addr, (void *) HumanoidMobRenderer_render_injection);

    // Disable Speed Bridging
    if (feature_has("Disable Speed Bridging", server_disabled)) {
        unsigned char disable_speed_bridging_patch[4] = {0x03, 0x00, 0x53, 0xe1}; // "cmp r3, r3"
        patch((void *) 0x494b4, disable_speed_bridging_patch);
    }

    // Disable Creative Mode Mining Delay
    if (feature_has("Disable Creative Mode Mining Delay", server_disabled)) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x19fa0, nop_patch);
    }

    // Init C++ And Logging
    _init_misc_cpp();
    _init_misc_logging();
    _init_misc_api();
}
