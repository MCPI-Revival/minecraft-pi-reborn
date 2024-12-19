#include <cmath>

#include <libreborn/patch.h>
#include <libreborn/env/env.h>
#include <libreborn/util/util.h>

#include <symbols/minecraft.h>

#include <media-layer/core.h>

#include <GLES/gl.h>
#include <SDL/SDL.h>

#include <mods/feature/feature.h>
#include <mods/classic-ui/classic-ui.h>
#include <mods/misc/misc.h>

#include "misc-internal.h"

// Heart Food Overlay
static int heal_amount = 0, heal_amount_drawing = 0;
static void Gui_renderHearts_injection(Gui_renderHearts_t original, Gui *gui) {
    // Calculate heal_amount
    heal_amount = heal_amount_drawing = 0;
    Inventory *inventory = gui->minecraft->player->inventory;
    const ItemInstance *held_ii = inventory->getSelected();
    if (held_ii) {
        Item *held = Item::items[held_ii->id];
        if (held->isFood() && held_ii->id) {
            const int nutrition = ((FoodItem *) held)->nutrition;
            const int cur_health = gui->minecraft->player->health;
            const int heal_num = std::min(cur_health + nutrition, 20) - cur_health;
            heal_amount = heal_amount_drawing = heal_num;
        }
    }

    // Call Original Method
    original(gui);
}
#define PINK_HEART_FULL 70
#define PINK_HEART_HALF 79
static void Gui_renderHearts_GuiComponent_blit_overlay_empty_injection(GuiComponent *gui, const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2, const int32_t w1, const int32_t h1, const int32_t w2, const int32_t h2) {
    // Call Original Method
    get_blit_with_classic_hud_offset()(gui, x1, y1, x2, y2, w1, h1, w2, h2);
    // Render The Overlay
    if (heal_amount_drawing == 1) {
        // Half Heart
        get_blit_with_classic_hud_offset()(gui, x1, y1, PINK_HEART_HALF, 0, w1, h1, w2, h2);
        heal_amount_drawing = 0;
    } else if (heal_amount_drawing > 0) {
        // Full Heart
        get_blit_with_classic_hud_offset()(gui, x1, y1, PINK_HEART_FULL, 0, w1, h1, w2, h2);
        heal_amount_drawing -= 2;
    }
}
static void Gui_renderHearts_GuiComponent_blit_overlay_hearts_injection(GuiComponent *gui, const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2, const int32_t w1, const int32_t h1, const int32_t w2, const int32_t h2) {
    // Offset the overlay
    if (x2 == 52) {
        heal_amount_drawing += 2;
    } else if (x2 == 61 && heal_amount) {
        // Half heart, flipped
        get_blit_with_classic_hud_offset()(gui, x1, y1, PINK_HEART_FULL, 0, w1, h1, w2, h2);
        heal_amount_drawing += 1;
    }
    // Call Original Method
    get_blit_with_classic_hud_offset()(gui, x1, y1, x2, y2, w1, h1, w2, h2);
    heal_amount_drawing = std::min(heal_amount_drawing, heal_amount);
}

// Additional GUI Rendering
static bool hide_chat_messages = false;
bool is_in_chat = false;
static bool render_selected_item_text = false;
static void Gui_renderChatMessages_injection(Gui_renderChatMessages_t original, Gui *gui, int32_t y_offset, const uint32_t max_messages, const bool disable_fading, Font *font) {
    // Handle Classic HUD
    y_offset -= get_classic_hud_y_offset(gui->minecraft);

    // Call Original Method
    if (!hide_chat_messages && (!is_in_chat || disable_fading)) {
        original(gui, y_offset, max_messages, disable_fading, font);
    }

    // Render Selected Item Text
    if (render_selected_item_text && !disable_fading) {
        // Fix GL Mode
        media_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Calculate Selected Item Text Scale
        const Minecraft *minecraft = gui->minecraft;
        const int32_t screen_width = minecraft->screen_width;
        const float scale = float(screen_width) * Gui::InvGuiScale;
        // Render Selected Item Text
        gui->renderOnSelectItemNameText((int32_t) scale, font, y_offset - 0x13);
    }
}
// Reset Selected Item Text Timer On Slot Select
static bool reset_selected_item_text_timer = false;
static void Gui_tick_injection(Gui_tick_t original, Gui *gui) {
    // Call Original Method
    original(gui);

    // Handle Reset
    if (render_selected_item_text && reset_selected_item_text_timer) {
        // Reset
        gui->selected_item_text_timer = 0;
        reset_selected_item_text_timer = false;
    }
}
// Trigger Reset Selected Item Text Timer On Slot Select
static void Inventory_selectSlot_injection(Inventory_selectSlot_t original, Inventory *inventory, const int32_t slot) {
    // Call Original Method
    original(inventory, slot);

    // Trigger Reset Selected Item Text Timer
    if (render_selected_item_text) {
        reset_selected_item_text_timer = true;
    }
}

// Translucent Toolbar
static void Gui_renderToolBar_injection(Gui_renderToolBar_t original, Gui *gui, const float param_1, const int32_t param_2, const int32_t param_3) {
    // Call Original Method
    media_glEnable(GL_BLEND);
    media_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    original(gui, param_1, param_2, param_3);
    media_glDisable(GL_BLEND);
}
static void Gui_renderToolBar_glColor4f_injection(const GLfloat red, const GLfloat green, const GLfloat blue, __attribute__((unused)) GLfloat alpha) {
    // Fix Alpha
    media_glColor4f(red, green, blue, 1.0f);
}

// Fix Screen Rendering When GUI is Hidden
static void Screen_render_injection(Screen_render_t original, Screen *screen, const int32_t param_1, const int32_t param_2, const float param_3) {
    // Fix
    media_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Call Original Method
    original(screen, param_1, param_2, param_3);
}

// Close Current Screen On Death To Prevent Bugs
static void LocalPlayer_die_injection(LocalPlayer_die_t original, LocalPlayer *entity, Entity *cause) {
    // Close Screen
    Minecraft *minecraft = entity->minecraft;
    minecraft->setScreen(nullptr);

    // Call Original Method
    original(entity, cause);
}

// Custom Cursor Rendering
//
// The default behavior for Touch GUI is to only render the cursor when the mouse is clicking, this fixes that.
// This also makes the cursor always render if the mouse is unlocked, instead of just when there is a Screen showing.
static void GameRenderer_render_injection(GameRenderer_render_t original, GameRenderer *game_renderer, const float param_1) {
    // Call Original Method
    original(game_renderer, param_1);

    // Check If Cursor Should Render
    if (media_SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF) {
        // Fix GL Mode
        media_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Get X And Y
        float x = Mouse::getX() * Gui::InvGuiScale;
        float y = Mouse::getY() * Gui::InvGuiScale;
        // Render Cursor
        Minecraft *minecraft = game_renderer->minecraft;
        Common::renderCursor(x, y, minecraft);
    }
}

// Fix Furnace Visual Bug
static int FurnaceTileEntity_getLitProgress_injection(FurnaceTileEntity_getLitProgress_t original, FurnaceTileEntity *furnace, const int max) {
    // Call Original Method
    int ret = original(furnace, max);

    // Fix Bug
    if (ret > max) {
        ret = max;
    }

    // Return
    return ret;
}

// Add Missing Buttons To Pause Menu
static void PauseScreen_init_injection(PauseScreen_init_t original, PauseScreen *screen) {
    // Call Original Method
    original(screen);

    // Check If Server
    const Minecraft *minecraft = screen->minecraft;
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

// Click Buttons On Mouse Down
static void Screen_mouseClicked_injection(__attribute__((unused)) Screen_mouseClicked_t original, Screen *self,  int x, int y, const int param_1) {
    if (param_1 == 1) {
        for (Button *button : self->rendered_buttons) {
            if (button->clicked(self->minecraft, x, y)) {
                // Click
                button->setPressed();
                self->clicked_button = button;
                self->buttonClicked(button);
                // Play Sound
                self->minecraft->sound_engine->playUI("random.click", 1, 1);
            }
        }
    }
}
static void Screen_mouseReleased_injection(__attribute__((unused)) Screen_mouseReleased_t original, Screen *self,  int x, int y, const int param_1) {
    if (param_1 == 1 && self->clicked_button) {
        self->clicked_button->released(x, y);
        self->clicked_button = nullptr;
    }
}
// Fix Exiting Pause Screen
static void Minecraft_grabMouse_injection(Minecraft_grabMouse_t original, Minecraft *self) {
    original(self);
    self->miss_time = 10000;
}
static void Minecraft_handleMouseDown_injection(Minecraft_handleMouseDown_t original, Minecraft *self, int param_1, bool can_destroy) {
    // Call Original Method
    original(self, param_1, can_destroy);
    // Reset Miss Time
    if (!can_destroy) {
        self->miss_time = 0;
    }
}

// Open Sign Screen
static void LocalPlayer_openTextEdit_injection(__attribute__((unused)) LocalPlayer_openTextEdit_t original, LocalPlayer *local_player, TileEntity *sign) {
    if (sign->type == 4) {
        Minecraft *minecraft = local_player->minecraft;
        TextEditScreen *screen = TextEditScreen::allocate();
        screen = screen->constructor((SignTileEntity *) sign);
        minecraft->setScreen((Screen *) screen);
    }
}

// Better GUI Scaling
static void set_gui_scale(const float new_scale) {
    union {
        float a;
        void *b;
    } pun = {};
    pun.a = new_scale;
    patch_address((void *) 0x17520, pun.b);
}
static float calculate_scale(const float value, const float default_value) {
    // y = mx + b
    const std::pair point_one = {default_value, 2.25f};
    const std::pair point_two = {default_value * 2, 4.5f};
    const float slope = (point_one.second - point_two.second) / (point_one.first - point_two.first);
    const float intercept = point_one.second - (slope * point_one.first);
    // Calculate
    float scale = (slope * value) + intercept;
    scale = std::round(scale);
    scale = std::max(scale, 1.0f);
    return scale;
}
static void Minecraft_setSize_injection(Minecraft_setSize_t original, Minecraft *self, const int width, const int height) {
    // Calculate Scale
    const float a = calculate_scale(float(width), DEFAULT_WIDTH);
    const float b = calculate_scale(float(height), DEFAULT_HEIGHT);
    set_gui_scale(std::min(a, b));
    // Call Original Method
    original(self, width, height);
}

// Batch Font Rendering
template <typename... Args>
static void Font_draw_injection(const std::function<void(Font *, Args...)> &original, Font *self, Args... args) {
    Tesselator &t = Tesselator::instance;
    const bool was_already_overridden = t.void_begin_end;
    if (!was_already_overridden) {
        t.begin(GL_QUADS);
        t.voidBeginAndEndCalls(true);
    }
    original(self, std::forward<Args>(args)...);
    if (!was_already_overridden) {
        t.voidBeginAndEndCalls(false);
        t.draw();
    }
}

// Screen Overlay
static void ItemInHandRenderer_renderScreenEffect_injection(__attribute__((unused)) ItemInHandRenderer_renderScreenEffect_t original, ItemInHandRenderer *self, float param_1) {
    media_glDisable(GL_ALPHA_TEST);
    const Minecraft *mc = self->minecraft;
    LocalPlayer *player = mc->player;
    mc->textures->loadAndBindTexture("terrain.png");
    if (player->isInWall()) {
        int x = Mth::floor(player->x);
        int y = Mth::floor(player->y);
        int z = Mth::floor(player->z);
        const int id = mc->level->getTile(x, y, z);
        Tile *tile = Tile::tiles[id];
        if (tile != nullptr) {
            media_glClear(GL_DEPTH_BUFFER_BIT);
            self->renderTex(param_1, tile->getTexture1(2));
        }
    }
    if (player->isOnFire()) {
        media_glClear(GL_DEPTH_BUFFER_BIT);
        self->renderFire(param_1);
    }
    media_glEnable(GL_ALPHA_TEST);
}

// Init
void _init_misc_ui() {
    // Food Overlay
    if (feature_has("Food Overlay", server_disabled)) {
        overwrite_calls(Gui_renderHearts, Gui_renderHearts_injection);
        overwrite_call((void *) 0x266f8, GuiComponent_blit, Gui_renderHearts_GuiComponent_blit_overlay_empty_injection);
        overwrite_call((void *) 0x267c8, GuiComponent_blit, Gui_renderHearts_GuiComponent_blit_overlay_hearts_injection);
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
        overwrite_call_manual((void *) 0x26c5c, (void *) Gui_renderToolBar_glColor4f_injection);
    }

    // Fix Screen Rendering When GUI is Hidden
    if (feature_has("Fix Screen Rendering When Hiding HUD", server_disabled)) {
        overwrite_calls(Screen_render, Screen_render_injection);
    }

    // Close Current Screen On Death To Prevent Bugs
    if (feature_has("Close Current Screen On Death", server_disabled)) {
        overwrite_calls(LocalPlayer_die, LocalPlayer_die_injection);
    }

    // Improved Cursor Rendering
    if (feature_has("Improved Cursor Rendering", server_disabled)) {
        // Disable Normal Cursor Rendering
        unsigned char disable_cursor_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x4a6c0, disable_cursor_patch);
        // Add Custom Cursor Rendering
        overwrite_calls(GameRenderer_render, GameRenderer_render_injection);
    }

    // Fix Furnace Visual Bug
    if (feature_has("Fix Furnace Screen Visual Bug", server_disabled)) {
        overwrite_calls(FurnaceTileEntity_getLitProgress, FurnaceTileEntity_getLitProgress_injection);
    }

    // Fix Pause Menu
    if (feature_has("Fix Pause Menu", server_disabled)) {
        // Add Missing Buttons To Pause Menu
        overwrite_calls(PauseScreen_init, PauseScreen_init_injection);
    }

    // Remove Forced GUI Lag
    if (feature_has("Remove Forced UI Lag (Can Break Joining Servers)", server_enabled)) {
        overwrite_calls(Common_sleepMs, nop<Common_sleepMs_t, int>);
    }

    // Custom GUI Scale
    bool patch_gui_scaling = false;
    const char *gui_scale_str = getenv(MCPI_GUI_SCALE_ENV);
    if (gui_scale_str != nullptr) {
        float gui_scale;
        env_value_to_obj(gui_scale, gui_scale_str);
        if (gui_scale > 0) {
            patch_gui_scaling = true;
            set_gui_scale(gui_scale);
        }
    }
    if (feature_has("Improved UI Scaling", server_disabled) && !patch_gui_scaling) {
        overwrite_calls(Minecraft_setSize, Minecraft_setSize_injection);
        patch_gui_scaling = true;
    }
    if (patch_gui_scaling) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x173e8, nop_patch);
        patch((void *) 0x173f0, nop_patch);
    }

    // Text Bugs
    if (feature_has("Text Rendering Fixes", server_disabled)) {
        // Don't Wrap Text On '\r' Or '\t' Because They Are Actual Characters In MCPI
        patch_address(&Strings::text_wrapping_delimiter, (void *) " \n");
        // Fix Width Of "Masculine Ordinal Indicator"
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x24d6c, nop_patch);
        patch((void *) 0x24d70, nop_patch);
        patch((void *) 0x24d74, nop_patch);
    }

    // Fix Invalid ItemInHandRenderer Texture Cache
    if (feature_has("Fix Held Item Caching", server_disabled)) {
        // This works by forcing MCPI to always use the branch that enables using the
        // cache, but then patches that to do the opposite.
        uchar ensure_equal_patch[] = {0x07, 0x00, 0x57, 0xe1}; // "cmp r7, r7"
        patch((void *) 0x4b938, ensure_equal_patch);
        uchar set_true_patch[] = {0x01, 0x30, 0xa0, 0x03}; // "moveq r3, #0x1"
        patch((void *) 0x4b93c, set_true_patch);
    }

    // Click Buttons On Mouse Down
    if (feature_has("Click Buttons On Mouse Down", server_disabled)) {
        overwrite_calls(Screen_mouseClicked, Screen_mouseClicked_injection);
        overwrite_calls(Screen_mouseReleased, Screen_mouseReleased_injection);
        overwrite_calls(Minecraft_grabMouse, Minecraft_grabMouse_injection);
        overwrite_calls(Minecraft_handleMouseDown, Minecraft_handleMouseDown_injection);
    }

    // Signs
    if (feature_has("Enable Sign Screen", server_disabled)) {
        // Fix Signs
        overwrite_calls(LocalPlayer_openTextEdit, LocalPlayer_openTextEdit_injection);
    }

    // Batch Font Rendering
    if (feature_has("Batch Font Rendering", server_disabled)) {
        overwrite_calls(Font_drawSlow, Font_draw_injection<const char *, float, float, unsigned int, bool>);
        overwrite_calls(Font_drawShadow, Font_draw_injection<const std::string &, float, float, unsigned int>);
        overwrite_calls(Font_drawShadow_raw, Font_draw_injection<const char *, float, float, unsigned int>);
    }

    // Fix Screen Overlays
    if (feature_has("Fix Held Item Poking Through Screen Overlay", server_disabled)) {
        overwrite_calls(ItemInHandRenderer_renderScreenEffect, ItemInHandRenderer_renderScreenEffect_injection);
    }
}
