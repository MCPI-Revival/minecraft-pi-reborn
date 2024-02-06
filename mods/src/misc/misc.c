#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>

#ifndef MCPI_HEADLESS_MODE
#include <GLES/gl.h>
#endif

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <SDL/SDL.h>
#include <media-layer/core.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include "misc-internal.h"
#include <mods/misc/misc.h>

// Heart food overlay
static int heal_amount = 0, heal_amount_drawing = 0;
void Gui_renderHearts_injection(Gui *gui) {
    // Get heal_amount
    heal_amount = heal_amount_drawing = 0;

    Inventory *inventory = gui->minecraft->player->inventory;
    ItemInstance *held_ii = Inventory_getSelected(inventory);
    if (held_ii) {
        Item *held = Item_items[held_ii->id];
        if (held->vtable->isFood(held) && held_ii->id) {
            int nutrition = ((FoodItem *) held)->nutrition;
            int cur_health = gui->minecraft->player->health;
            int heal_num = fmin(cur_health + nutrition, 20) - cur_health;
            heal_amount = heal_amount_drawing = heal_num;
        }
    }

    // Call original
    Gui_renderHearts(gui);
}

Gui_blit_t Gui_blit_renderHearts_injection = NULL;
void Gui_renderHearts_GuiComponent_blit_overlay_empty_injection(Gui *gui, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t w1, int32_t h1, int32_t w2, int32_t h2) {
    // Call original
    Gui_blit_renderHearts_injection(gui, x1, y1, x2, y2, w1, h1, w2, h2);
    // Render the overlay
    if (heal_amount_drawing == 1) {
        // Half heart
        Gui_blit_renderHearts_injection(gui, x1, y1, 79, 0, w1, h1, w2, h2);
        heal_amount_drawing = 0;
    } else if (heal_amount_drawing > 0) {
        // Full heart
        Gui_blit_renderHearts_injection(gui, x1, y1, 70, 0, w1, h1, w2, h2);
        heal_amount_drawing -= 2;
    }
}

void Gui_renderHearts_GuiComponent_blit_overlay_hearts_injection(Gui *gui, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t w1, int32_t h1, int32_t w2, int32_t h2) {
    // Offset the overlay
    if (x2 == 52) {
        heal_amount_drawing += 2;
    } else if (x2 == 61 && heal_amount) {
        // Half heart, flipped
        Gui_blit_renderHearts_injection(gui, x1, y1, 70, 0, w1, h1, w2, h2);
        heal_amount_drawing += 1;
    };
    // Call original
    Gui_blit_renderHearts_injection(gui, x1, y1, x2, y2, w1, h1, w2, h2);
    heal_amount_drawing = fmin(heal_amount_drawing, heal_amount);
}

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
static void Gui_renderHearts_GuiComponent_blit_hearts_injection(Gui *component, int32_t x_dest, int32_t y_dest, int32_t x_src, int32_t y_src, int32_t width_dest, int32_t height_dest, int32_t width_src, int32_t height_src) {
    Minecraft *minecraft = component->minecraft;
    x_dest -= DEFAULT_HUD_PADDING;
    float width = ((float) minecraft->screen_width) * Gui_InvGuiScale;
    float height = ((float) minecraft->screen_height) * Gui_InvGuiScale;
    x_dest += (width - (NUMBER_OF_SLOTS * SLOT_WIDTH)) / 2;
    y_dest -= DEFAULT_HUD_PADDING;
    y_dest += height - HUD_ELEMENT_HEIGHT - TOOLBAR_HEIGHT - NEW_HUD_PADDING;
    // Call Original Method
    Gui_blit(component, x_dest, y_dest, x_src, y_src, width_dest, height_dest, width_src, height_src);
}
static void Gui_renderHearts_GuiComponent_blit_armor_injection(Gui *component, int32_t x_dest, int32_t y_dest, int32_t x_src, int32_t y_src, int32_t width_dest, int32_t height_dest, int32_t width_src, int32_t height_src) {
    Minecraft *minecraft = component->minecraft;
    x_dest -= DEFAULT_HUD_PADDING + HUD_ELEMENT_WIDTH;
    float width = ((float) minecraft->screen_width) * Gui_InvGuiScale;
    float height = ((float) minecraft->screen_height) * Gui_InvGuiScale;
    x_dest += width - ((width - (NUMBER_OF_SLOTS * SLOT_WIDTH)) / 2) - HUD_ELEMENT_WIDTH;
    y_dest -= DEFAULT_HUD_PADDING;
    y_dest += height - HUD_ELEMENT_HEIGHT - TOOLBAR_HEIGHT - NEW_HUD_PADDING;
    // Call Original Method
    Gui_blit(component, x_dest, y_dest, x_src, y_src, width_dest, height_dest, width_src, height_src);
}
static void Gui_renderBubbles_GuiComponent_blit_injection(Gui *component, int32_t x_dest, int32_t y_dest, int32_t x_src, int32_t y_src, int32_t width_dest, int32_t height_dest, int32_t width_src, int32_t height_src) {
    Minecraft *minecraft = component->minecraft;
    x_dest -= DEFAULT_HUD_PADDING;
    float width = ((float) minecraft->screen_width) * Gui_InvGuiScale;
    float height = ((float) minecraft->screen_height) * Gui_InvGuiScale;
    x_dest += (width - (NUMBER_OF_SLOTS * SLOT_WIDTH)) / 2;
    y_dest -= DEFAULT_HUD_PADDING + DEFAULT_BUBBLES_PADDING + HUD_ELEMENT_HEIGHT;
    y_dest += height - HUD_ELEMENT_HEIGHT - TOOLBAR_HEIGHT - HUD_ELEMENT_HEIGHT - NEW_HUD_PADDING;
    // Call Original Method
    Gui_blit(component, x_dest, y_dest, x_src, y_src, width_dest, height_dest, width_src, height_src);
}

// Additional GUI Rendering
static int hide_chat_messages = 0;
bool is_in_chat = 0;
static int render_selected_item_text = 0;
static void Gui_renderChatMessages_injection(Gui *gui, int32_t y_offset, uint32_t max_messages, bool disable_fading, Font *font) {
    // Handle Classic HUD
    if (use_classic_hud) {
        Minecraft *minecraft = gui->minecraft;
        if (!Minecraft_isCreativeMode(minecraft)) {
            y_offset -= (HUD_ELEMENT_HEIGHT * 2) + NEW_HUD_PADDING;
        }
    }

    // Call Original Method
    if (!hide_chat_messages && !is_in_chat) {
        Gui_renderChatMessages(gui, y_offset, max_messages, disable_fading, font);
    }

    // Render Selected Item Text
    if (render_selected_item_text) {
        // Fix GL Mode
#ifndef MCPI_HEADLESS_MODE
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
        // Calculate Selected Item Text Scale
        Minecraft *minecraft = gui->minecraft;
        int32_t screen_width = minecraft->screen_width;
        float scale = ((float) screen_width) * Gui_InvGuiScale;
        // Render Selected Item Text
        Gui_renderOnSelectItemNameText(gui, (int32_t) scale, font, y_offset - 0x13);
    }
}
// Reset Selected Item Text Timer On Slot Select
static uint32_t reset_selected_item_text_timer = 0;
static void Gui_tick_injection(Gui *gui) {
    // Call Original Method
    Gui_tick(gui);

    // Handle Reset
    if (render_selected_item_text) {
        float *selected_item_text_timer = &gui->selected_item_text_timer;
        if (reset_selected_item_text_timer) {
            // Reset
            *selected_item_text_timer = 0;
            reset_selected_item_text_timer = 0;
        }
    }
}
// Trigger Reset Selected Item Text Timer On Slot Select
static void Inventory_selectSlot_injection(Inventory *inventory, int32_t slot) {
    // Call Original Method
    Inventory_selectSlot(inventory, slot);

    // Trigger Reset Selected Item Text Timer
    if (render_selected_item_text) {
        reset_selected_item_text_timer = 1;
    }
}

// Translucent Toolbar
static void Gui_renderToolBar_injection(Gui *gui, float param_1, int32_t param_2, int32_t param_3) {
    // Call Original Method
#ifndef MCPI_HEADLESS_MODE
    int was_blend_enabled = glIsEnabled(GL_BLEND);
    if (!was_blend_enabled) {
        glEnable(GL_BLEND);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
    Gui_renderToolBar(gui, param_1, param_2, param_3);
#ifndef MCPI_HEADLESS_MODE
    if (!was_blend_enabled) {
        glDisable(GL_BLEND);
    }
#endif
}
static void Gui_renderToolBar_glColor4f_injection(GLfloat red, GLfloat green, GLfloat blue, __attribute__((unused)) GLfloat alpha) {
    // Fix Alpha
#ifndef MCPI_HEADLESS_MODE
    glColor4f(red, green, blue, 1.0f);
#else
    (void) red;
    (void) green;
    (void) blue;
#endif
}

// Fix Screen Rendering When GUI is Hidden
static void Screen_render_injection(Screen *screen, int32_t param_1, int32_t param_2, float param_3) {
    // Fix
#ifndef MCPI_HEADLESS_MODE
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
    // Call Original Method
    Screen_render_non_virtual(screen, param_1, param_2, param_3);
}

// Sanitize Username
#define MAX_USERNAME_LENGTH 16
static void LoginPacket_read_injection(LoginPacket *packet, RakNet_BitStream *bit_stream) {
    // Call Original Method
    LoginPacket_read_non_virtual(packet, bit_stream);

    // Prepare
    RakNet_RakString *rak_string = &packet->username;
    // Get Original Username
    RakNet_RakString_SharedString *shared_string = rak_string->sharedString;
    char *c_str = shared_string->c_str;
    // Sanitize
    char *new_username = strdup(c_str);
    ALLOC_CHECK(new_username);
    sanitize_string(&new_username, MAX_USERNAME_LENGTH, 0);
    // Set New Username
    RakNet_RakString_Assign(rak_string, new_username);
    // Free
    free(new_username);
}

// Fix RakNet::RakString Security Bug
//
// RakNet::RakString's format constructor is often given unsanitized user input and is never used for formatting,
// this is a massive security risk, allowing clients to run arbitrary format specifiers, this disables the
// formatting functionality.
static RakNet_RakString *RakNet_RakString_injection(RakNet_RakString *rak_string, const char *format, ...) {
    // Call Original Method
    return RakNet_RakString_constructor(rak_string, "%s", format);
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
static RakNet_StartupResult RakNetInstance_host_RakNet_RakPeer_Startup_injection(RakNet_RakPeer *rak_peer, unsigned short maxConnections, unsigned char *socketDescriptors, uint32_t socketDescriptorCount, int32_t threadPriority) {
    // Call Original Method
    RakNet_StartupResult result = rak_peer->vtable->Startup(rak_peer, maxConnections, socketDescriptors, socketDescriptorCount, threadPriority);

    // Print Error
    if (result != RAKNET_STARTED) {
        PRINT_RAKNET_STARTUP_FAILURE("Failed To Start RakNet: %s", RAKNET_ERROR_NAMES[result]);
    }

    // Return
    return result;
}

// Fix Bug Where RakNetInstance Starts Pinging Potential Servers Before The "Join Game" Screen Is Opened
static RakNetInstance *RakNetInstance_injection(RakNetInstance *rak_net_instance) {
    // Call Original Method
    RakNetInstance *result = RakNetInstance_constructor(rak_net_instance);
    // Fix
    rak_net_instance->pinging_for_hosts = 0;
    // Return
    return result;
}

// Close Current Screen On Death To Prevent Bugs
static void LocalPlayer_die_injection(LocalPlayer *entity, Entity *cause) {
    // Close Screen
    Minecraft *minecraft = entity->minecraft;
    Minecraft_setScreen(minecraft, NULL);

    // Call Original Method
    LocalPlayer_die_non_virtual(entity, cause);
}

// Fix Furnace Not Checking Item Auxiliary When Inserting New Item
static int32_t FurnaceScreen_handleAddItem_injection(FurnaceScreen *furnace_screen, int32_t slot, ItemInstance *item) {
    // Get Existing Item
    FurnaceTileEntity *tile_entity = furnace_screen->tile_entity;
    ItemInstance *existing_item = tile_entity->vtable->getItem(tile_entity, slot);

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
        return FurnaceScreen_handleAddItem(furnace_screen, slot, item);
    } else {
        // Invalid
        return 0;
    }
}

// Custom Cursor Rendering
//
// The default behavior for Touch GUI is to only render the cursor when the mouse is clicking, this fixes that.
// This also makes the cursor always render if the mouse is unlocked, instead of just when there is a Screen showing.
#ifndef MCPI_HEADLESS_MODE
static void GameRenderer_render_injection(GameRenderer *game_renderer, float param_1) {
    // Call Original Method
    GameRenderer_render(game_renderer, param_1);

    // Check If Cursor Should Render
    if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF) {
        // Fix GL Mode
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Get X And Y
        float x = Mouse_getX() * Gui_InvGuiScale;
        float y = Mouse_getY() * Gui_InvGuiScale;
        // Render Cursor
        Minecraft *minecraft = game_renderer->minecraft;
        Common_renderCursor(x, y, minecraft);
    }
}
#endif

// Get Real Selected Slot
int32_t misc_get_real_selected_slot(Player *player) {
    // Get Selected Slot
    Inventory *inventory = player->inventory;
    int32_t selected_slot = inventory->selectedSlot;

    // Linked Slots
    int32_t linked_slots_length = inventory->linked_slots_length;
    if (selected_slot < linked_slots_length) {
        int32_t *linked_slots = inventory->linked_slots;
        selected_slot = linked_slots[selected_slot];
    }

    // Return
    return selected_slot;
}

#ifndef MCPI_HEADLESS_MODE
// Properly Generate Buffers
static void anGenBuffers_injection(int32_t count, uint32_t *buffers) {
    glGenBuffers(count, buffers);
}
#endif

// Fix Graphics Bug When Switching To First-Person While Sneaking
static void HumanoidMobRenderer_render_injection(HumanoidMobRenderer *model_renderer, Entity *entity, float param_2, float param_3, float param_4, float param_5, float param_6) {
    HumanoidMobRenderer_render_non_virtual(model_renderer, entity, param_2, param_3, param_4, param_5, param_6);
    HumanoidModel *model = model_renderer->model;
    model->is_sneaking = 0;
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
    return real_bind(sockfd, new_addr, addrlen);
}

// Change Grass Color
static int32_t get_color(LevelSource *level_source, int32_t x, int32_t z) {
    Biome *biome = level_source->vtable->getBiome(level_source, x, z);
    if (biome == NULL) {
        return 0;
    }
    return biome->color;
}
#define BIOME_BLEND_SIZE 7
static int32_t GrassTile_getColor_injection(__attribute__((unused)) Tile *tile, LevelSource *level_source, int32_t x, __attribute__((unused)) int32_t y, int32_t z) {
    int r_sum = 0;
    int g_sum = 0;
    int b_sum = 0;
    int color_sum = 0;
    int x_start = x - (BIOME_BLEND_SIZE / 2);
    int z_start = z - (BIOME_BLEND_SIZE / 2);
    for (int x_offset = 0; x_offset < BIOME_BLEND_SIZE; x_offset++) {
        for (int z_offset = 0; z_offset < BIOME_BLEND_SIZE; z_offset++) {
            int32_t color = get_color(level_source, x_start + x_offset, z_start + z_offset);
            r_sum += (color >> 16) & 0xff;
            g_sum += (color >> 8) & 0xff;
            b_sum += color & 0xff;
            color_sum++;
        }
    }
    int r_avg = r_sum / color_sum;
    int g_avg = g_sum / color_sum;
    int b_avg = b_sum / color_sum;
    return (r_avg << 16) | (g_avg << 8) | b_avg;
}
static int32_t TallGrass_getColor_injection(TallGrass *tile, LevelSource *level_source, int32_t x, int32_t y, int32_t z) {
    int32_t original_color = TallGrass_getColor_non_virtual(tile, level_source, x, y, z);
    if (original_color == 0x339933) {
        return GrassTile_getColor_injection((Tile *) tile, level_source, x, y, z);
    } else {
        return original_color;
    }
}

// Generate Caves
static void RandomLevelSource_buildSurface_injection(RandomLevelSource *random_level_source, int32_t chunk_x, int32_t chunk_y, unsigned char *chunk_data, Biome **biomes) {
    // Call Original Method
    RandomLevelSource_buildSurface(random_level_source, chunk_x, chunk_y, chunk_data, biomes);

    // Get Level
    Level *level = random_level_source->level;

    // Get Cave Feature
    LargeCaveFeature *cave_feature = &random_level_source->cave_feature;

    // Generate
    cave_feature->vtable->apply(cave_feature, (ChunkSource *) random_level_source, level, chunk_x, chunk_y, chunk_data, 0);
}

// No Block Tinting
static int32_t Tile_getColor_injection() {
    return 0xffffff;
}

// Disable Hostile AI In Creative Mode
#define has_vtable(obj, type) (((void *) obj->vtable) == type##_vtable_base)
static Entity *PathfinderMob_findAttackTarget_injection(PathfinderMob *mob) {
    // Call Original Method
    Entity *target = mob->vtable->findAttackTarget(mob);

    // Check If Creative Mode
    if (target != NULL) {
        bool is_player = has_vtable(target, Player) || has_vtable(target, LocalPlayer) || has_vtable(target, ServerPlayer) || has_vtable(target, RemotePlayer);
        if (is_player) {
            Player *player = (Player *) target;
            Inventory *inventory = player->inventory;
            bool is_creative = inventory->is_creative;
            if (is_creative) {
                target = NULL;
            }
        }
    }

    // Return
    return target;
}

// 3D Chests
static int32_t Tile_getRenderShape_injection(Tile *tile) {
    if (tile == Tile_chest) {
        // Don't Render "Simple" Chest Model
        return -1;
    } else {
        // Call Original Method
        return tile->vtable->getRenderShape(tile);
    }
}
static ChestTileEntity *ChestTileEntity_injection(ChestTileEntity *tile_entity) {
    // Call Original Method
    ChestTileEntity_constructor(tile_entity);

    // Enable Renderer
    tile_entity->renderer_id = 1;

    // Return
    return tile_entity;
}
static bool is_rendering_chest = 0;
static void ModelPart_render_injection(ModelPart *model_part, float scale) {
    // Start
    is_rendering_chest = 1;

    // Call Original Method
    ModelPart_render(model_part, scale);

    // Stop
    is_rendering_chest = 0;
}
static void Tesselator_vertexUV_injection(Tesselator *tesselator, float x, float y, float z, float u, float v) {
    // Fix Chest Texture
    if (is_rendering_chest) {
        v /= 2;
    }

    // Call Original Method
    Tesselator_vertexUV(tesselator, x, y, z, u, v);
}
static bool ChestTileEntity_shouldSave_injection(__attribute__((unused)) unsigned char *tile_entity) {
    return 1;
}

// Animated 3D Chest
static ContainerMenu *ContainerMenu_injection(ContainerMenu *container_menu, Container *container, int32_t param_1) {
    // Call Original Method
    ContainerMenu_constructor(container_menu, container, param_1);

    // Play Animation
    ChestTileEntity *tile_entity = (ChestTileEntity *) (((unsigned char *) container) - offsetof(ChestTileEntity, container));
    bool is_client = tile_entity->is_client;
    if (!is_client) {
        container->vtable->startOpen(container);
    }

    // Return
    return container_menu;
}
static ContainerMenu *ContainerMenu_destructor_injection(ContainerMenu *container_menu) {
    // Play Animation
    Container *container = container_menu->container;
    ChestTileEntity *tile_entity = (ChestTileEntity *) (((unsigned char *) container) - offsetof(ChestTileEntity, container));
    bool is_client = tile_entity->is_client;
    if (!is_client) {
        container->vtable->stopOpen(container);
    }

    // Call Original Method
    return ContainerMenu_destructor_complete_non_virtual(container_menu);
}

#ifndef MCPI_HEADLESS_MODE
// Custom Outline Color
static void glColor4f_injection(__attribute__((unused)) GLfloat red, __attribute__((unused)) GLfloat green, __attribute__((unused)) GLfloat blue, __attribute__((unused)) GLfloat alpha) {
    // Set Color
    glColor4f(0, 0, 0, 0.4);

    // Find Line Width
    char *custom_line_width = getenv("MCPI_BLOCK_OUTLINE_WIDTH");
    float line_width;
    if (custom_line_width != NULL) {
        // Custom
        line_width = strtof(custom_line_width, NULL);
    } else {
        // Guess
        line_width = 2 / Gui_InvGuiScale;
    }
    // Clamp Line Width
    float range[2];
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, range);
    if (range[1] < line_width) {
        line_width = range[1];
    } else if (range[0] > line_width) {
        line_width = range[0];
    }
    // Set Line Width
    glLineWidth(line_width);
}
#endif

// Fix Furnace Visual Bug
static int FurnaceTileEntity_getLitProgress_injection(FurnaceTileEntity *furnace, int max) {
    // Call Original Method
    int ret = FurnaceTileEntity_getLitProgress(furnace, max);

    // Fix Bug
    if (ret > max) {
        ret = max;
    }

    // Return
    return ret;
}

// Java Light Ramp
static void Dimension_updateLightRamp_injection(Dimension *self) {
    // https://github.com/ReMinecraftPE/mcpe/blob/d7a8b6baecf8b3b050538abdbc976f690312aa2d/source/world/level/Dimension.cpp#L92-L105
    for (int i = 0; i <= 15; i++) {
        float f1 = 1.0f - (((float) i) / 15.0f);
        self->light_ramp[i] = ((1.0f - f1) / (f1 * 3.0f + 1.0f)) * (1.0f - 0.1f) + 0.1f;
        // Default Light Ramp:
        // float fVar4 = 1.0 - ((float) i * 0.0625);
        // self->light_ramp[i] = ((1.0 - fVar4) / (fVar4 * 3.0 + 1.0)) * 0.95 + 0.15;
    }
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

    // Food overlay
    char food_overlay = 0;
    Gui_blit_renderHearts_injection = Gui_blit;
    if (feature_has("Food Overlay", server_disabled)) {
        food_overlay = 1;
        overwrite_calls((void *) Gui_renderHearts, Gui_renderHearts_injection);
        overwrite_call((void *) 0x266f8, (void *) Gui_renderHearts_GuiComponent_blit_overlay_empty_injection);
        overwrite_call((void *) 0x267c8, (void *) Gui_renderHearts_GuiComponent_blit_overlay_hearts_injection);
    }

    // Classic HUD
    if (feature_has("Classic HUD", server_disabled)) {
        use_classic_hud = 1;
        overwrite_call((void *) 0x26758, (void *) Gui_renderHearts_GuiComponent_blit_hearts_injection);
        overwrite_call((void *) 0x2656c, (void *) Gui_renderHearts_GuiComponent_blit_armor_injection);
        overwrite_call((void *) 0x268c4, (void *) Gui_renderBubbles_GuiComponent_blit_injection);
        if (!food_overlay) {
            overwrite_call((void *) 0x266f8, (void *) Gui_renderHearts_GuiComponent_blit_hearts_injection);
            overwrite_call((void *) 0x267c8, (void *) Gui_renderHearts_GuiComponent_blit_hearts_injection);
        }
        Gui_blit_renderHearts_injection = Gui_renderHearts_GuiComponent_blit_hearts_injection;
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
    overwrite_calls((void *) Screen_render_non_virtual, (void *) Screen_render_injection);

    // Sanitize Username
    patch_address(LoginPacket_read_vtable_addr, (void *) LoginPacket_read_injection);

    // Fix RakNet::RakString Security Bug
    overwrite_calls((void *) RakNet_RakString_constructor, (void *) RakNet_RakString_injection);

    // Print Error Message If RakNet Startup Fails
    overwrite_call((void *) 0x73778, (void *) RakNetInstance_host_RakNet_RakPeer_Startup_injection);

    // Fix Bug Where RakNetInstance Starts Pinging Potential Servers Before The "Join Game" Screen Is Opened
    overwrite_calls((void *) RakNetInstance_constructor, (void *) RakNetInstance_injection);

    // Close Current Screen On Death To Prevent Bugs
    if (feature_has("Close Current Screen On Death", server_disabled)) {
        patch_address(LocalPlayer_die_vtable_addr, (void *) LocalPlayer_die_injection);
    }

    // Fix Furnace Not Checking Item Auxiliary When Inserting New Item
    if (feature_has("Fix Furnace Not Checking Item Auxiliary", server_disabled)) {
        overwrite_calls((void *) FurnaceScreen_handleAddItem, (void *) FurnaceScreen_handleAddItem_injection);
    }

#ifdef MCPI_HEADLESS_MODE
    // Don't Render Game In Headless Mode
    overwrite_calls((void *) GameRenderer_render, (void *) nop);
    overwrite_calls((void *) NinecraftApp_initGLStates, (void *) nop);
    overwrite_calls((void *) Gui_onConfigChanged, (void *) nop);
    overwrite_calls((void *) LevelRenderer_generateSky, (void *) nop);
#else
    // Improved Cursor Rendering
    if (feature_has("Improved Cursor Rendering", server_disabled)) {
        // Disable Normal Cursor Rendering
        unsigned char disable_cursor_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x4a6c0, disable_cursor_patch);
        // Add Custom Cursor Rendering
        overwrite_calls((void *) GameRenderer_render, (void *) GameRenderer_render_injection);
    }
#endif

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
        overwrite_calls((void *) Common_sleepMs, (void *) nop);
    }

#ifndef MCPI_HEADLESS_MODE
    // Properly Generate Buffers
    overwrite((void *) Common_anGenBuffers, (void *) anGenBuffers_injection);
#endif

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

    // Change Grass Color
    if (feature_has("Add Biome Colors To Grass", server_disabled)) {
        patch_address((void *) GrassTile_getColor_vtable_addr, (void *) GrassTile_getColor_injection);
        patch_address((void *) TallGrass_getColor_vtable_addr, (void *) TallGrass_getColor_injection);
    }

    // Generate Caves
    if (feature_has("Generate Caves", server_auto)) {
        overwrite_calls((void *) RandomLevelSource_buildSurface, (void *) RandomLevelSource_buildSurface_injection);
    }

    // Disable Block Tinting
    if (feature_has("Disable Block Tinting", server_disabled)) {
        patch_address((void *) GrassTile_getColor_vtable_addr, (void *) Tile_getColor_injection);
        patch_address((void *) TallGrass_getColor_vtable_addr, (void *) Tile_getColor_injection);
        patch_address((void *) StemTile_getColor_vtable_addr, (void *) Tile_getColor_injection);
        patch_address((void *) LeafTile_getColor_vtable_addr, (void *) Tile_getColor_injection);
        overwrite((void *) LiquidTile_getColor_non_virtual, (void *) Tile_getColor_injection);
    }

    // Custom GUI Scale
    const char *gui_scale_str = getenv("MCPI_GUI_SCALE");
    if (gui_scale_str != NULL) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x173e8, nop_patch);
        patch((void *) 0x173f0, nop_patch);
        float gui_scale = strtof(gui_scale_str, NULL);
        uint32_t gui_scale_raw;
        memcpy(&gui_scale_raw, &gui_scale, sizeof (gui_scale_raw));
        patch_address((void *) 0x17520, (void *) gui_scale_raw);
    }

    // Disable Hostile AI In Creative Mode
    if (feature_has("Disable Hostile AI In Creative Mode", server_enabled)) {
        overwrite_call((void *) 0x83b8c, (void *) PathfinderMob_findAttackTarget_injection);
    }

    // 3D Chests
    if (feature_has("3D Chest Model", server_disabled)) {
        overwrite_call((void *) 0x5e830, (void *) Tile_getRenderShape_injection);
        overwrite_calls((void *) ChestTileEntity_constructor, (void *) ChestTileEntity_injection);
        overwrite_call((void *) 0x6655c, (void *) ModelPart_render_injection);
        overwrite_call((void *) 0x66568, (void *) ModelPart_render_injection);
        overwrite_call((void *) 0x66574, (void *) ModelPart_render_injection);
        overwrite_calls((void *) Tesselator_vertexUV, (void *) Tesselator_vertexUV_injection);
        unsigned char chest_model_patch[4] = {0x13, 0x20, 0xa0, 0xe3}; // "mov r2, #0x13"
        patch((void *) 0x66fc8, chest_model_patch);
        unsigned char chest_color_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x66404, chest_color_patch);

        // Animation
        overwrite_calls((void *) ContainerMenu_constructor, (void *) ContainerMenu_injection);
        overwrite_calls((void *) ContainerMenu_destructor_complete_non_virtual, (void *) ContainerMenu_destructor_injection);
        patch_address(ContainerMenu_destructor_complete_vtable_addr, (void *) ContainerMenu_destructor_injection);
    }
    patch_address((void *) 0x115b48, (void *) ChestTileEntity_shouldSave_injection);

#ifndef MCPI_HEADLESS_MODE
    // Replace Block Highlight With Outline
    if (feature_has("Replace Block Highlight With Outline", server_disabled)) {
        overwrite((void *) LevelRenderer_renderHitSelect, (void *) LevelRenderer_renderHitOutline);
        unsigned char fix_outline_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x4d830, fix_outline_patch);
        overwrite_call((void *) 0x4d764, (void *) glColor4f_injection);
    }
#endif

    // Fix Furnace Visual Bug
    overwrite_calls((void *) FurnaceTileEntity_getLitProgress, (void *) FurnaceTileEntity_getLitProgress_injection);

    // Send the full level, not only changed chunks
    if (feature_has("Send Full Level When Hosting Game", server_enabled)) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x717c4, nop_patch);
        unsigned char mov_r3_ff[4] = {0xff, 0x30, 0xa0, 0xe3}; // "mov r3, #0xff"
        patch((void *) 0x7178c, mov_r3_ff);
    }

    // Java Light Ramp
    if (feature_has("Use Java Beta 1.3 Light Ramp", server_disabled)) {
        overwrite((void *) Dimension_updateLightRamp_non_virtual, (void *) Dimension_updateLightRamp_injection);
    }

    // Init C++ And Logging
    _init_misc_cpp();
    _init_misc_logging();
    _init_misc_api();
}
