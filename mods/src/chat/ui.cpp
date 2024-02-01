// Config Needs To Load First
#include <libreborn/libreborn.h>

// Chat UI Code Is Useless In Headless Mode
#ifndef MCPI_HEADLESS_MODE

#include "chat-internal.h"
#include <mods/chat/chat.h>
#include <mods/text-input-box/TextInputScreen.h>
#include <mods/misc/misc.h>

// Structure
struct ChatScreen {
    TextInputScreen super;
    TextInputBox chat;
};
CUSTOM_VTABLE(chat_screen, Screen) {
    TextInputScreen::setup(vtable);
    // Init
    vtable->init = [](Screen *super) {
        Screen_init_non_virtual(super);
        ChatScreen *self = (ChatScreen *) super;
        self->super.m_textInputs.push_back(&self->chat);
        self->chat.init(super->font);
        self->chat.setFocused(true);
        is_in_chat = true;
    };
    // Removal
    vtable->removed = [](Screen *super) {
        Screen_removed_non_virtual(super);
        is_in_chat = false;
    };
    // Rendering
    static Screen_render_t original_render = vtable->render;
    vtable->render = [](Screen *super, int x, int y, float param_1) {
        // Background
        super->vtable->renderBackground(super);
        // Render Chat
        Gui_renderChatMessages(&super->minecraft->gui, super->height, 20, true, super->font);
        // Call Original Method
        original_render(super, x, y, param_1);
    };
    // Positioning
    vtable->setupPositions = [](Screen *super) {
        Screen_setupPositions_non_virtual(super);
        ChatScreen *self = (ChatScreen *) super;
        int height = 20;
        int x = 0;
        int y = super->height - height;
        int width = super->width;
        self->chat.setSize(x, y, width, height);
    };
    // Key Presses
    static Screen_keyPressed_t original_keyPressed = vtable->keyPressed;
    vtable->keyPressed = [](Screen *super, int key) {
        // Handle Enter
        ChatScreen *self = (ChatScreen *) super;
        if (key == 0x0d) {
            _chat_queue_message(self->chat.m_text.c_str());
            Minecraft_setScreen(super->minecraft, NULL);
        }
        // Call Original Method
        original_keyPressed(super, key);
    };
}
static Screen *create_chat_screen() {
    // Construct
    ChatScreen *screen = new ChatScreen;
    ALLOC_CHECK(screen);
    Screen_constructor(&screen->super.super);

    // Set VTable
    screen->super.super.vtable = get_chat_screen_vtable();

    // Setup
    screen->chat = TextInputBox::create(0);

    // Return
    return (Screen *) screen;
}

// Open Screen
static bool open_chat_screen = false;
void chat_open() {
    open_chat_screen = true;
}

// Init
void _init_chat_ui() {
    misc_run_on_tick([](Minecraft *minecraft) {
        if (open_chat_screen && Minecraft_isLevelGenerated(minecraft) && minecraft->screen == NULL) {
            Minecraft_setScreen(minecraft, create_chat_screen());
        }
        open_chat_screen = false;
    });
}

#endif
