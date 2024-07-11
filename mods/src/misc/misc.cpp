#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cmath>
#include <string>
#include <fstream>
#include <streambuf>
#include <algorithm>

#include <GLES/gl.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <SDL/SDL.h>
#include <media-layer/core.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include <mods/input/input.h>
#include <mods/misc/misc.h>

#include "misc-internal.h"

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
static void Gui_renderHearts_GuiComponent_blit_hearts_injection(GuiComponent *component, int32_t x_dest, int32_t y_dest, int32_t x_src, int32_t y_src, int32_t width_dest, int32_t height_dest, int32_t width_src, int32_t height_src) {
    Minecraft *minecraft = ((Gui *) component)->minecraft;
    x_dest -= DEFAULT_HUD_PADDING;
    float width = ((float) minecraft->screen_width) * Gui::InvGuiScale;
    float height = ((float) minecraft->screen_height) * Gui::InvGuiScale;
    x_dest += (width - (NUMBER_OF_SLOTS * SLOT_WIDTH)) / 2;
    y_dest -= DEFAULT_HUD_PADDING;
    y_dest += height - HUD_ELEMENT_HEIGHT - TOOLBAR_HEIGHT - NEW_HUD_PADDING;
    // Call Original Method
    component->blit(x_dest, y_dest, x_src, y_src, width_dest, height_dest, width_src, height_src);
}
static void Gui_renderHearts_GuiComponent_blit_armor_injection(Gui *component, int32_t x_dest, int32_t y_dest, int32_t x_src, int32_t y_src, int32_t width_dest, int32_t height_dest, int32_t width_src, int32_t height_src) {
    Minecraft *minecraft = component->minecraft;
    x_dest -= DEFAULT_HUD_PADDING + HUD_ELEMENT_WIDTH;
    float width = ((float) minecraft->screen_width) * Gui::InvGuiScale;
    float height = ((float) minecraft->screen_height) * Gui::InvGuiScale;
    x_dest += width - ((width - (NUMBER_OF_SLOTS * SLOT_WIDTH)) / 2) - HUD_ELEMENT_WIDTH;
    y_dest -= DEFAULT_HUD_PADDING;
    y_dest += height - HUD_ELEMENT_HEIGHT - TOOLBAR_HEIGHT - NEW_HUD_PADDING;
    // Call Original Method
    component->blit(x_dest, y_dest, x_src, y_src, width_dest, height_dest, width_src, height_src);
}
static void Gui_renderBubbles_GuiComponent_blit_injection(Gui *component, int32_t x_dest, int32_t y_dest, int32_t x_src, int32_t y_src, int32_t width_dest, int32_t height_dest, int32_t width_src, int32_t height_src) {
    Minecraft *minecraft = component->minecraft;
    x_dest -= DEFAULT_HUD_PADDING;
    float width = ((float) minecraft->screen_width) * Gui::InvGuiScale;
    float height = ((float) minecraft->screen_height) * Gui::InvGuiScale;
    x_dest += (width - (NUMBER_OF_SLOTS * SLOT_WIDTH)) / 2;
    y_dest -= DEFAULT_HUD_PADDING + DEFAULT_BUBBLES_PADDING + HUD_ELEMENT_HEIGHT;
    y_dest += height - HUD_ELEMENT_HEIGHT - TOOLBAR_HEIGHT - HUD_ELEMENT_HEIGHT - NEW_HUD_PADDING;
    // Call Original Method
    component->blit(x_dest, y_dest, x_src, y_src, width_dest, height_dest, width_src, height_src);
}

// Heart Food Overlay
static int heal_amount = 0, heal_amount_drawing = 0;
static void Gui_renderHearts_injection(Gui_renderHearts_t original, Gui *gui) {
    // Get heal_amount
    heal_amount = heal_amount_drawing = 0;

    Inventory *inventory = gui->minecraft->player->inventory;
    ItemInstance *held_ii = inventory->getSelected();
    if (held_ii) {
        Item *held = Item::items[held_ii->id];
        if (held->isFood() && held_ii->id) {
            int nutrition = ((FoodItem *) held)->nutrition;
            int cur_health = gui->minecraft->player->health;
            int heal_num = fmin(cur_health + nutrition, 20) - cur_health;
            heal_amount = heal_amount_drawing = heal_num;
        }
    }

    // Call original
    original(gui);
}
static GuiComponent_blit_t get_blit_with_classic_hud_offset() {
    return use_classic_hud ? Gui_renderHearts_GuiComponent_blit_hearts_injection : GuiComponent_blit;
}
#define PINK_HEART_FULL 70
#define PINK_HEART_HALF 79
static void Gui_renderHearts_GuiComponent_blit_overlay_empty_injection(Gui *gui, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t w1, int32_t h1, int32_t w2, int32_t h2) {
    // Call Original Method
    get_blit_with_classic_hud_offset()((GuiComponent *) gui, x1, y1, x2, y2, w1, h1, w2, h2);
    // Render The Overlay
    if (heal_amount_drawing == 1) {
        // Half Heart
        get_blit_with_classic_hud_offset()((GuiComponent *) gui, x1, y1, PINK_HEART_HALF, 0, w1, h1, w2, h2);
        heal_amount_drawing = 0;
    } else if (heal_amount_drawing > 0) {
        // Full Heart
        get_blit_with_classic_hud_offset()((GuiComponent *) gui, x1, y1, PINK_HEART_FULL, 0, w1, h1, w2, h2);
        heal_amount_drawing -= 2;
    }
}
static void Gui_renderHearts_GuiComponent_blit_overlay_hearts_injection(Gui *gui, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t w1, int32_t h1, int32_t w2, int32_t h2) {
    // Offset the overlay
    if (x2 == 52) {
        heal_amount_drawing += 2;
    } else if (x2 == 61 && heal_amount) {
        // Half heart, flipped
        get_blit_with_classic_hud_offset()((GuiComponent *) gui, x1, y1, PINK_HEART_FULL, 0, w1, h1, w2, h2);
        heal_amount_drawing += 1;
    }
    // Call Original Method
    get_blit_with_classic_hud_offset()((GuiComponent *) gui, x1, y1, x2, y2, w1, h1, w2, h2);
    heal_amount_drawing = fmin(heal_amount_drawing, heal_amount);
}

// Additional GUI Rendering
static int hide_chat_messages = 0;
bool is_in_chat = false;
static int render_selected_item_text = 0;
static void Gui_renderChatMessages_injection(Gui_renderChatMessages_t original, Gui *gui, int32_t y_offset, uint32_t max_messages, bool disable_fading, Font *font) {
    // Handle Classic HUD
    if (use_classic_hud) {
        Minecraft *minecraft = gui->minecraft;
        if (!Minecraft_isCreativeMode(minecraft)) {
            y_offset -= (HUD_ELEMENT_HEIGHT * 2) + NEW_HUD_PADDING;
        }
    }

    // Call Original Method
    if (!hide_chat_messages && (!is_in_chat || disable_fading)) {
        original(gui, y_offset, max_messages, disable_fading, font);
    }

    // Render Selected Item Text
    if (render_selected_item_text && !disable_fading) {
        // Fix GL Mode
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Calculate Selected Item Text Scale
        Minecraft *minecraft = gui->minecraft;
        int32_t screen_width = minecraft->screen_width;
        float scale = ((float) screen_width) * Gui::InvGuiScale;
        // Render Selected Item Text
        gui->renderOnSelectItemNameText((int32_t) scale, font, y_offset - 0x13);
    }
}
// Reset Selected Item Text Timer On Slot Select
static uint32_t reset_selected_item_text_timer = 0;
static void Gui_tick_injection(Gui_tick_t original, Gui *gui) {
    // Call Original Method
    original(gui);

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
static void Inventory_selectSlot_injection(Inventory_selectSlot_t original, Inventory *inventory, int32_t slot) {
    // Call Original Method
    original(inventory, slot);

    // Trigger Reset Selected Item Text Timer
    if (render_selected_item_text) {
        reset_selected_item_text_timer = 1;
    }
}

// Translucent Toolbar
static void Gui_renderToolBar_injection(Gui_renderToolBar_t original, Gui *gui, float param_1, int32_t param_2, int32_t param_3) {
    // Call Original Method
    bool was_blend_enabled = glIsEnabled(GL_BLEND);
    if (!was_blend_enabled) {
        glEnable(GL_BLEND);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    original(gui, param_1, param_2, param_3);
    if (!was_blend_enabled) {
        glDisable(GL_BLEND);
    }
}
static void Gui_renderToolBar_glColor4f_injection(GLfloat red, GLfloat green, GLfloat blue, __attribute__((unused)) GLfloat alpha) {
    // Fix Alpha
    glColor4f(red, green, blue, 1.0f);
}

// Fix Screen Rendering When GUI is Hidden
static void Screen_render_injection(Screen_render_t original, Screen *screen, int32_t param_1, int32_t param_2, float param_3) {
    // Fix
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Call Original Method
    original(screen, param_1, param_2, param_3);
}

// Sanitize Username
#define MAX_USERNAME_LENGTH 16
static void LoginPacket_read_injection(LoginPacket_read_t original, LoginPacket *packet, RakNet_BitStream *bit_stream) {
    // Call Original Method
    original(packet, bit_stream);

    // Prepare
    RakNet_RakString *rak_string = &packet->username;
    // Get Original Username
    RakNet_RakString_SharedString *shared_string = rak_string->sharedString;
    char *c_str = shared_string->c_str;
    // Sanitize
    char *new_username = strdup(c_str);
    ALLOC_CHECK(new_username);
    sanitize_string(new_username, MAX_USERNAME_LENGTH, 0);
    // Set New Username
    rak_string->Assign(new_username);
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
static const char *RAKNET_ERROR_NAMES[] = {
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
static RakNet_StartupResult RakNetInstance_host_RakNet_RakPeer_Startup_injection(RakNet_RakPeer *rak_peer, unsigned short maxConnections, unsigned char *socketDescriptors, uint32_t socketDescriptorCount, int32_t threadPriority) {
    // Call Original Method
    RakNet_StartupResult result = rak_peer->Startup(maxConnections, socketDescriptors, socketDescriptorCount, threadPriority);

    // Print Error
    if (result != RAKNET_STARTED) {
        CONDITIONAL_ERR(reborn_is_server(), "Failed To Start RakNet: %s", RAKNET_ERROR_NAMES[result]);
    }

    // Return
    return result;
}

// Fix Bug Where RakNetInstance Starts Pinging Potential Servers Before The "Join Game" Screen Is Opened
static RakNetInstance *RakNetInstance_injection(RakNetInstance_constructor_t original, RakNetInstance *rak_net_instance) {
    // Call Original Method
    RakNetInstance *result = original(rak_net_instance);
    // Fix
    rak_net_instance->pinging_for_hosts = 0;
    // Return
    return result;
}

// Close Current Screen On Death To Prevent Bugs
static void LocalPlayer_die_injection(LocalPlayer_die_t original, LocalPlayer *entity, Entity *cause) {
    // Close Screen
    Minecraft *minecraft = entity->minecraft;
    minecraft->setScreen(nullptr);

    // Call Original Method
    original(entity, cause);
}

// Fix Furnace Not Checking Item Auxiliary When Inserting New Item
static int32_t FurnaceScreen_handleAddItem_injection(FurnaceScreen_handleAddItem_t original, FurnaceScreen *furnace_screen, int32_t slot, ItemInstance *item) {
    // Get Existing Item
    FurnaceTileEntity *tile_entity = furnace_screen->tile_entity;
    ItemInstance *existing_item = tile_entity->getItem(slot);

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
        return original(furnace_screen, slot, item);
    } else {
        // Invalid
        return 0;
    }
}

// Custom Cursor Rendering
//
// The default behavior for Touch GUI is to only render the cursor when the mouse is clicking, this fixes that.
// This also makes the cursor always render if the mouse is unlocked, instead of just when there is a Screen showing.
static void GameRenderer_render_injection(GameRenderer_render_t original, GameRenderer *game_renderer, float param_1) {
    // Call Original Method
    original(game_renderer, param_1);

    // Check If Cursor Should Render
    if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF) {
        // Fix GL Mode
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Get X And Y
        float x = Mouse::getX() * Gui::InvGuiScale;
        float y = Mouse::getY() * Gui::InvGuiScale;
        // Render Cursor
        Minecraft *minecraft = game_renderer->minecraft;
        Common::renderCursor(x, y, minecraft);
    }
}

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

// Properly Generate Buffers
static void anGenBuffers_injection(__attribute__((unused)) Common_anGenBuffers_t original, const int32_t count, uint32_t *buffers) {
    if (!reborn_is_headless()) {
        glGenBuffers(count, buffers);
    }
}

// Fix Graphics Bug When Switching To First-Person While Sneaking
static void PlayerRenderer_render_injection(PlayerRenderer *model_renderer, Entity *entity, float param_2, float param_3, float param_4, float param_5, float param_6) {
    (*HumanoidMobRenderer_render_vtable_addr)((HumanoidMobRenderer *) model_renderer, entity, param_2, param_3, param_4, param_5, param_6);
    HumanoidModel *model = model_renderer->model;
    model->is_sneaking = false;
}

// Custom API Port
HOOK(bind, int, (int sockfd, const struct sockaddr *addr, socklen_t addrlen)) {
    const sockaddr *new_addr = addr;
    sockaddr_in in_addr = {};
    if (addr->sa_family == AF_INET) {
        in_addr = *(const struct sockaddr_in *) new_addr;
        if (in_addr.sin_port == ntohs(4711)) {
            const char *new_port_str = getenv(MCPI_API_PORT_ENV);
            long int new_port;
            if (new_port_str != nullptr && (new_port = strtol(new_port_str, nullptr, 0)) != 0L) {
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
    Biome *biome = level_source->getBiome(x, z);
    if (biome == nullptr) {
        return 0;
    }
    return biome->color;
}
#define BIOME_BLEND_SIZE 7
static int32_t GrassTile_getColor_injection(__attribute__((unused)) GrassTile_getColor_t original, __attribute__((unused)) GrassTile *tile, LevelSource *level_source, int32_t x, __attribute__((unused)) int32_t y, int32_t z) {
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
static int32_t TallGrass_getColor_injection(TallGrass_getColor_t original, TallGrass *tile, LevelSource *level_source, int32_t x, int32_t y, int32_t z) {
    int32_t original_color = original(tile, level_source, x, y, z);
    if (original_color == 0x339933) {
        return GrassTile_getColor_injection(nullptr, nullptr, level_source, x, y, z);
    } else {
        return original_color;
    }
}

// Generate Caves
static void RandomLevelSource_buildSurface_injection(RandomLevelSource_buildSurface_t original, RandomLevelSource *random_level_source, int32_t chunk_x, int32_t chunk_y, unsigned char *chunk_data, Biome **biomes) {
    // Call Original Method
    original(random_level_source, chunk_x, chunk_y, chunk_data, biomes);

    // Get Level
    Level *level = random_level_source->level;

    // Get Cave Feature
    LargeCaveFeature *cave_feature = &random_level_source->cave_feature;

    // Generate
    cave_feature->apply((ChunkSource *) random_level_source, level, chunk_x, chunk_y, chunk_data, 0);
}

// No Block Tinting
template <typename T>
static int32_t Tile_getColor_injection(__attribute__((unused)) std::function<int(T *, LevelSource *, int, int, int)> original, __attribute__((unused)) T *self, __attribute__((unused)) LevelSource *level_source, __attribute__((unused)) int x, __attribute__((unused)) int y, __attribute__((unused)) int z) {
    return 0xffffff;
}

// Disable Hostile AI In Creative Mode
static Entity *PathfinderMob_findAttackTarget_injection(PathfinderMob *mob) {
    // Call Original Method
    Entity *target = mob->findAttackTarget();

    // Only modify the AI of monsters
    if (mob->getCreatureBaseType() != 1) {
        return target;
    }

    // Check If Creative Mode
    if (target != nullptr && target->isPlayer()) {
        Player *player = (Player *) target;
        Inventory *inventory = player->inventory;
        bool is_creative = inventory->is_creative;
        if (is_creative) {
            target = nullptr;
        }
    }

    // Return
    return target;
}

// 3D Chests
static int32_t Tile_getRenderShape_injection(Tile *tile) {
    if (tile == Tile::chest) {
        // Don't Render "Simple" Chest Model
        return -1;
    } else {
        // Call Original Method
        return tile->getRenderShape();
    }
}
static ChestTileEntity *ChestTileEntity_injection(ChestTileEntity_constructor_t original, ChestTileEntity *tile_entity) {
    // Call Original Method
    original(tile_entity);

    // Enable Renderer
    tile_entity->renderer_id = 1;

    // Return
    return tile_entity;
}
static bool is_rendering_chest = false;
static void ModelPart_render_injection(ModelPart *model_part, float scale) {
    // Start
    is_rendering_chest = true;

    // Call Original Method
    model_part->render(scale);

    // Stop
    is_rendering_chest = false;
}
static void Tesselator_vertexUV_injection(Tesselator_vertexUV_t original, Tesselator *tesselator, float x, float y, float z, float u, float v) {
    // Fix Chest Texture
    if (is_rendering_chest) {
        v /= 2;
    }

    // Call Original Method
    original(tesselator, x, y, z, u, v);
}
static bool ChestTileEntity_shouldSave_injection(__attribute__((unused)) ChestTileEntity_shouldSave_t original, __attribute__((unused)) ChestTileEntity *tile_entity) {
    return true;
}

// Animated 3D Chest
static ContainerMenu *ContainerMenu_injection(ContainerMenu_constructor_t original, ContainerMenu *container_menu, Container *container, int32_t param_1) {
    // Call Original Method
    original(container_menu, container, param_1);

    // Play Animation
    ChestTileEntity *tile_entity = (ChestTileEntity *) (((unsigned char *) container) - offsetof(ChestTileEntity, container));
    bool is_client = tile_entity->is_client;
    if (!is_client) {
        container->startOpen();
    }

    // Return
    return container_menu;
}
static ContainerMenu *ContainerMenu_destructor_injection(ContainerMenu_destructor_complete_t original, ContainerMenu *container_menu) {
    // Play Animation
    Container *container = container_menu->container;
    ChestTileEntity *tile_entity = (ChestTileEntity *) (((unsigned char *) container) - offsetof(ChestTileEntity, container));
    bool is_client = tile_entity->is_client;
    if (!is_client) {
        container->stopOpen();
    }

    // Call Original Method
    return original(container_menu);
}

// Custom Outline Color
static void LevelRenderer_render_AABB_glColor4f_injection(__attribute__((unused)) GLfloat red, __attribute__((unused)) GLfloat green, __attribute__((unused)) GLfloat blue, __attribute__((unused)) GLfloat alpha) {
    // Set Color
    glColor4f(0, 0, 0, 0.4);

    // Find Line Width
    char *custom_line_width = getenv(MCPI_BLOCK_OUTLINE_WIDTH_ENV);
    float line_width;
    if (custom_line_width != nullptr) {
        // Custom
        line_width = strtof(custom_line_width, nullptr);
    } else {
        // Guess
        line_width = 1.5f / Gui::InvGuiScale;
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

// Fix Furnace Visual Bug
static int FurnaceTileEntity_getLitProgress_injection(FurnaceTileEntity_getLitProgress_t original, FurnaceTileEntity *furnace, int max) {
    // Call Original Method
    int ret = original(furnace, max);

    // Fix Bug
    if (ret > max) {
        ret = max;
    }

    // Return
    return ret;
}

// Fix used items transferring durability
static int selected_slot = -1;
static void Player_startUsingItem_injection(Player_startUsingItem_t original, Player *self, ItemInstance *item_instance, int time) {
    selected_slot = self->inventory->selectedSlot;
    original(self, item_instance, time);
}
static void Player_stopUsingItem_injection(Player_stopUsingItem_t original, Player *self) {
    if (selected_slot != self->inventory->selectedSlot) {
        self->itemBeingUsed.id = 0;
    }
    original(self);
}

// Java Light Ramp
static void Dimension_updateLightRamp_injection(__attribute__((unused)) Dimension_updateLightRamp_t original, Dimension *self) {
    // https://github.com/ReMinecraftPE/mcpe/blob/d7a8b6baecf8b3b050538abdbc976f690312aa2d/source/world/level/Dimension.cpp#L92-L105
    for (int i = 0; i <= 15; i++) {
        float f1 = 1.0f - (((float) i) / 15.0f);
        self->light_ramp[i] = ((1.0f - f1) / (f1 * 3.0f + 1.0f)) * (1.0f - 0.1f) + 0.1f;
        // Default Light Ramp:
        // float fVar4 = 1.0 - ((float) i * 0.0625);
        // self->light_ramp[i] = ((1.0 - fVar4) / (fVar4 * 3.0 + 1.0)) * 0.95 + 0.15;
    }
}

// Read Asset File
static AppPlatform_readAssetFile_return_value AppPlatform_readAssetFile_injection(__attribute__((unused)) AppPlatform_readAssetFile_t original, __attribute__((unused)) AppPlatform *app_platform, std::string *path) {
    // Open File
    std::ifstream stream("data/" + *path, std::ios_base::binary | std::ios_base::ate);
    if (!stream) {
        // Does Not Exist
        AppPlatform_readAssetFile_return_value ret;
        ret.length = -1;
        ret.data = nullptr;
        return ret;
    }
    // Read File
    std::streamoff len = stream.tellg();
    char *buf = new char[len];
    ALLOC_CHECK(buf);
    stream.seekg(0, std::ifstream::beg);
    stream.read(buf, len);
    stream.close();
    // Return String
    AppPlatform_readAssetFile_return_value ret;
    ret.length = len;
    ret.data = strdup(buf);
    return ret;
}

// Add Missing Buttons To Pause Menu
static void PauseScreen_init_injection(PauseScreen_init_t original, PauseScreen *screen) {
    // Call Original Method
    original(screen);

    // Check If Server
    Minecraft *minecraft = screen->minecraft;
    RakNetInstance *rak_net_instance = minecraft->rak_net_instance;
    if (rak_net_instance != nullptr) {
        if (rak_net_instance->isServer()) {
            // Add Button
            std::vector<Button *> *rendered_buttons = &screen->rendered_buttons;
            std::vector<Button *> *selectable_buttons = &screen->selectable_buttons;
            Button *button = screen->server_visibility_button;
            rendered_buttons->push_back(button);
            selectable_buttons->push_back(button);

            // Update Button Text
            screen->updateServerVisibilityText();
        }
    }
}

// Implement crafting remainders
void PaneCraftingScreen_craftSelectedItem_PaneCraftingScreen_recheckRecipes_injection(PaneCraftingScreen *self) {
    // Check for crafting remainders
    CItem *item = self->item;
    for (size_t i = 0; i < item->ingredients.size(); i++) {
        ItemInstance requested_item_instance = item->ingredients[i].requested_item;
        Item *requested_item = Item::items[requested_item_instance.id];
        ItemInstance *craftingRemainingItem = requested_item->getCraftingRemainingItem(&requested_item_instance);
        if (craftingRemainingItem != nullptr) {
            // Add or drop remainder
            LocalPlayer *player = self->minecraft->player;
            if (!player->inventory->add(craftingRemainingItem)) {
                // Drop
                player->drop(craftingRemainingItem, false);
            }
        }
    }
    // Call Original Method
    self->recheckRecipes();
}

ItemInstance *Item_getCraftingRemainingItem_injection(__attribute__((unused)) Item_getCraftingRemainingItem_t original, Item *self, ItemInstance *item_instance) {
    if (self->craftingRemainingItem != nullptr) {
        ItemInstance *ret = new ItemInstance;
        ret->id = self->craftingRemainingItem->id;
        ret->count = item_instance->count;
        ret->auxiliary = 0;
        return ret;
    }
    return nullptr;
}

// Sort Chunks
struct chunk_data {
    Chunk *chunk;
    float distance;
};
#define MAX_CHUNKS_SIZE 24336
static chunk_data data[MAX_CHUNKS_SIZE];
static void sort_chunks(Chunk **chunks_begin, Chunk **chunks_end, DistanceChunkSorter sorter) {
    // Calculate Distances
    int chunks_size = chunks_end - chunks_begin;
    if (chunks_size > MAX_CHUNKS_SIZE) {
        IMPOSSIBLE();
    }
    for (int i = 0; i < chunks_size; i++) {
        Chunk *chunk = chunks_begin[i];
        float distance = chunk->distanceToSqr((Entity *) sorter.mob);
        if ((1024.0 <= distance) && chunk->y < 0x40) {
            distance *= 10.0;
        }
        data[i].chunk = chunk;
        data[i].distance = distance;
    }

    // Sort
    std::sort(data, data + chunks_size, [](chunk_data &a, chunk_data &b) {
        return a.distance < b.distance;
    });
    for (int i = 0; i < chunks_size; i++) {
        chunks_begin[i] = data[i].chunk;
    }
}

// Display Date In Select World Screen
static std::string AppPlatform_linux_getDateString_injection(__attribute__((unused)) AppPlatform_linux *app_platform, int time) {
    // From https://github.com/ReMinecraftPE/mcpe/blob/56e51027b1c2e67fe5a0e8a091cefe51d4d11926/platforms/sdl/base/AppPlatform_sdl_base.cpp#L68-L84
    const time_t tt = time;
    tm t = {};
    gmtime_r(&tt, &t);
    char buf[2048];
    strftime(buf, sizeof buf, "%b %d %Y %H:%M:%S", &t);
    return std::string(buf);
}

// Init
template <typename... Args>
static void nop(__attribute__((unused)) Args... args) {
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
        overwrite_call((void *) 0x26758, (void *) Gui_renderHearts_GuiComponent_blit_hearts_injection);
        overwrite_call((void *) 0x2656c, (void *) Gui_renderHearts_GuiComponent_blit_armor_injection);
        overwrite_call((void *) 0x268c4, (void *) Gui_renderBubbles_GuiComponent_blit_injection);
        overwrite_call((void *) 0x266f8, (void *) Gui_renderHearts_GuiComponent_blit_hearts_injection);
        overwrite_call((void *) 0x267c8, (void *) Gui_renderHearts_GuiComponent_blit_hearts_injection);
    }

    // Food overlay
    if (feature_has("Food Overlay", server_disabled)) {
        overwrite_calls(Gui_renderHearts, Gui_renderHearts_injection);
        overwrite_call((void *) 0x266f8, (void *) Gui_renderHearts_GuiComponent_blit_overlay_empty_injection);
        overwrite_call((void *) 0x267c8, (void *) Gui_renderHearts_GuiComponent_blit_overlay_hearts_injection);
    }

    // Render Selected Item Text + Hide Chat Messages
    hide_chat_messages = feature_has("Hide Chat Messages", server_disabled);
    render_selected_item_text = feature_has("Render Selected Item Text", server_disabled);
    overwrite_calls(Gui_renderChatMessages, Gui_renderChatMessages_injection);
    overwrite_calls(Gui_tick, Gui_tick_injection);
    overwrite_calls(Inventory_selectSlot, Inventory_selectSlot_injection);

    // Translucent Toolbar
    if (feature_has("Translucent Toolbar", server_disabled)) {
        overwrite_calls(Gui_renderToolBar, Gui_renderToolBar_injection);
        overwrite_call((void *) 0x26c5c, (void *) Gui_renderToolBar_glColor4f_injection);
    }

    // Fix Screen Rendering When GUI is Hidden
    overwrite_calls(Screen_render, Screen_render_injection);

    // Sanitize Username
    overwrite_calls(LoginPacket_read, LoginPacket_read_injection);

    // Fix RakNet::RakString Security Bug
    overwrite_calls_manual((void *) RakNet_RakString_constructor, (void *) RakNet_RakString_injection);

    // Print Error Message If RakNet Startup Fails
    overwrite_call((void *) 0x73778, (void *) RakNetInstance_host_RakNet_RakPeer_Startup_injection);

    // Fix Bug Where RakNetInstance Starts Pinging Potential Servers Before The "Join Game" Screen Is Opened
    overwrite_calls(RakNetInstance_constructor, RakNetInstance_injection);

    // Close Current Screen On Death To Prevent Bugs
    if (feature_has("Close Current Screen On Death", server_disabled)) {
        overwrite_calls(LocalPlayer_die, LocalPlayer_die_injection);
    }

    // Fix Furnace Not Checking Item Auxiliary When Inserting New Item
    if (feature_has("Fix Furnace Not Checking Item Auxiliary", server_disabled)) {
        overwrite_calls(FurnaceScreen_handleAddItem, FurnaceScreen_handleAddItem_injection);
    }

    // Improved Cursor Rendering
    if (feature_has("Improved Cursor Rendering", server_disabled)) {
        // Disable Normal Cursor Rendering
        unsigned char disable_cursor_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x4a6c0, disable_cursor_patch);
        // Add Custom Cursor Rendering
        overwrite_calls(GameRenderer_render, GameRenderer_render_injection);
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
        overwrite_calls(Common_sleepMs, nop<Common_sleepMs_t, int>);
    }

    // Properly Generate Buffers
    overwrite_calls(Common_anGenBuffers, anGenBuffers_injection);

    // Fix Graphics Bug When Switching To First-Person While Sneaking
    patch_vtable(PlayerRenderer_render, PlayerRenderer_render_injection);

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
        overwrite_calls(GrassTile_getColor, GrassTile_getColor_injection);
        overwrite_calls(TallGrass_getColor, TallGrass_getColor_injection);
    }

    // Generate Caves
    if (feature_has("Generate Caves", server_auto)) {
        overwrite_calls(RandomLevelSource_buildSurface, RandomLevelSource_buildSurface_injection);
    }

    // Disable Block Tinting
    if (feature_has("Disable Block Tinting", server_disabled)) {
        overwrite_calls(GrassTile_getColor, Tile_getColor_injection<GrassTile>);
        overwrite_calls(TallGrass_getColor, Tile_getColor_injection<TallGrass>);
        overwrite_calls(StemTile_getColor, Tile_getColor_injection<StemTile>);
        overwrite_calls(LeafTile_getColor, Tile_getColor_injection<LeafTile>);
        overwrite_calls(LiquidTile_getColor, Tile_getColor_injection<LiquidTile>);
    }

    // Custom GUI Scale
    const char *gui_scale_str = getenv(MCPI_GUI_SCALE_ENV);
    if (gui_scale_str != nullptr) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x173e8, nop_patch);
        patch((void *) 0x173f0, nop_patch);
        float gui_scale = strtof(gui_scale_str, nullptr);
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
        overwrite_calls(ChestTileEntity_constructor, ChestTileEntity_injection);
        overwrite_call((void *) 0x6655c, (void *) ModelPart_render_injection);
        overwrite_call((void *) 0x66568, (void *) ModelPart_render_injection);
        overwrite_call((void *) 0x66574, (void *) ModelPart_render_injection);
        overwrite_calls(Tesselator_vertexUV, Tesselator_vertexUV_injection);
        unsigned char chest_model_patch[4] = {0x13, 0x20, 0xa0, 0xe3}; // "mov r2, #0x13"
        patch((void *) 0x66fc8, chest_model_patch);
        unsigned char chest_color_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x66404, chest_color_patch);

        // Animation
        overwrite_calls(ContainerMenu_constructor, ContainerMenu_injection);
        overwrite_calls(ContainerMenu_destructor_complete, ContainerMenu_destructor_injection);
    }
    overwrite_calls(ChestTileEntity_shouldSave, ChestTileEntity_shouldSave_injection);

    // Replace Block Highlight With Outline
    if (feature_has("Replace Block Highlight With Outline", server_disabled)) {
        overwrite_calls(LevelRenderer_renderHitSelect, [](__attribute__((unused)) LevelRenderer_renderHitSelect_t original, LevelRenderer *self, Player *player, HitResult *hit_result, int i, void *vp, float f) {
            self->renderHitOutline(player, hit_result, i, vp, f);
        });
        unsigned char fix_outline_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x4d830, fix_outline_patch);
        overwrite_call((void *) 0x4d764, (void *) LevelRenderer_render_AABB_glColor4f_injection);
    }

    // Fix Furnace Visual Bug
    overwrite_calls(FurnaceTileEntity_getLitProgress, FurnaceTileEntity_getLitProgress_injection);

    // Send the full level, not only changed chunks
    if (feature_has("Send Full Level When Hosting Game", server_enabled)) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x717c4, nop_patch);
        unsigned char mov_r3_ff[4] = {0xff, 0x30, 0xa0, 0xe3}; // "mov r3, #0xff"
        patch((void *) 0x7178c, mov_r3_ff);
    }

    // Java Light Ramp
    if (feature_has("Use Java Beta 1.3 Light Ramp", server_disabled)) {
        overwrite_calls(Dimension_updateLightRamp, Dimension_updateLightRamp_injection);
    }

    // Fix used items transferring durability
    overwrite_calls(Player_startUsingItem, Player_startUsingItem_injection);
    overwrite_calls(Player_stopUsingItem, Player_stopUsingItem_injection);

    // Fix invalid ItemInHandRenderer texture cache
    if (feature_has("Fix Held Item Caching", server_disabled)) {
        // This works by forcing MCPI to always use the branch that enables using the
        // cache, but then patches that as well to do the opposite
        uchar ensure_equal_patch[] = {0x07, 0x00, 0x57, 0xe1}; // "cmp r7, r7"
        patch((void *) 0x4b938, ensure_equal_patch);
        uchar set_true_patch[] = {0x01, 0x30, 0xa0, 0x03}; // "moveq r3, #0x1"
        patch((void *) 0x4b93c, set_true_patch);
    }

    // Implement AppPlatform::readAssetFile So Translations Work
    if (feature_has("Load Language Files", server_enabled)) {
        overwrite_calls(AppPlatform_readAssetFile, AppPlatform_readAssetFile_injection);
    }

    // Fix Pause Menu
    if (feature_has("Fix Pause Menu", server_disabled)) {
        // Add Missing Buttons To Pause Menu
        overwrite_calls(PauseScreen_init, PauseScreen_init_injection);
    }

    // Implement Crafting Remainders
    overwrite_call((void *) 0x2e230, (void *) PaneCraftingScreen_craftSelectedItem_PaneCraftingScreen_recheckRecipes_injection);
    overwrite_calls(Item_getCraftingRemainingItem, Item_getCraftingRemainingItem_injection);

    // Replace 2011 std::sort With Optimized(TM) Code
    if (feature_has("Optimized Chunk Sorting", server_enabled)) {
        overwrite_calls_manual((void *) 0x51fac, (void *) sort_chunks);
    }

    // Display Date In Select World Screen
    if (feature_has("Display Date In Select World Screen", server_disabled)) {
        patch_vtable(AppPlatform_linux_getDateString, AppPlatform_linux_getDateString_injection);
    }

    // Don't Wrap Text On '\r' Or '\t' Because THey Are Actual Characters In MCPI
    patch_address(&Strings::text_wrapping_delimiter, (void *) " \n");

    // Fullscreen
    misc_run_on_key_press([](__attribute__((unused)) Minecraft *mc, int key) {
        if (key == MC_KEY_F11) {
            media_toggle_fullscreen();
            return true;
        } else {
            return false;
        }
    });

    // Init Logging
    _init_misc_logging();
    _init_misc_api();

    // Don't Render Game In Headless Mode
    if (reborn_is_headless()) {
        overwrite_calls(GameRenderer_render, nop<GameRenderer_render_t, GameRenderer *, float>);
        overwrite_calls(NinecraftApp_initGLStates, nop<NinecraftApp_initGLStates_t, NinecraftApp *>);
        overwrite_calls(Gui_onConfigChanged, nop<Gui_onConfigChanged_t, Gui *, Config *>);
        overwrite_calls(LevelRenderer_generateSky, nop<LevelRenderer_generateSky_t, LevelRenderer *>);
    }
}
