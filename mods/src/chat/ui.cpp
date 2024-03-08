// Config Needs To Load First
#include <libreborn/libreborn.h>

// Chat UI Code Is Useless In Headless Mode
#ifndef MCPI_HEADLESS_MODE

#include "chat-internal.h"
#include <mods/chat/chat.h>
#include <mods/text-input-box/TextInputScreen.h>
#include <mods/misc/misc.h>
#include <mods/touch/touch.h>

std::vector<std::string> history;

// Structure
struct ChatScreen {
    TextInputScreen super;
    TextInputBox *chat;
    Button *send;
};
CUSTOM_VTABLE(chat_screen, Screen) {
    TextInputScreen::setup(vtable);
    // Init
    static Screen_init_t original_init = vtable->init;
    vtable->init = [](Screen *super) {
        original_init(super);
        ChatScreen *self = (ChatScreen *) super;
        // Text Input
        self->chat = TextInputBox::create("", "", &history);
        self->super.m_textInputs->push_back(self->chat);
        self->chat->init(super->font);
        self->chat->setFocused(true);
        // Determine Max Length
        std::string prefix = _chat_get_prefix(Strings_default_username);
        int max_length = MAX_CHAT_MESSAGE_LENGTH - prefix.length();
        self->chat->setMaxLength(max_length);
        // Send Button
        self->send = touch_create_button(1, "Send");
        super->rendered_buttons.push_back(self->send);
        super->selectable_buttons.push_back(self->send);
        // Hide Chat Messages
        is_in_chat = true;
    };
    // Removal
    static Screen_removed_t original_removed = vtable->removed;
    vtable->removed = [](Screen *super) {
        original_removed(super);
        is_in_chat = false;
        ChatScreen *self = (ChatScreen *) super;
        delete self->chat;
        self->send->vtable->destructor_deleting(self->send);
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
        self->send->height = 24;
        self->send->width = 40;
        int x = 0;
        int y = super->height - self->send->height;
        int width = super->width - self->send->width;
        self->chat->setSize(x, y, width, self->send->height);
        self->send->y = super->height - self->send->height;
        self->send->x = x + width;
    };
    // Key Presses
    static Screen_keyPressed_t original_keyPressed = vtable->keyPressed;
    vtable->keyPressed = [](Screen *super, int key) {
        // Handle Enter
        ChatScreen *self = (ChatScreen *) super;
        if (key == 0x0d && self->chat->isFocused()) {
            if (self->chat->getText().length() > 0) {
                std::string text = self->chat->getText();
                if (self->chat->history_pos != int(history.size() - 1)) {
                    history.push_back(text);
                }
                _chat_queue_message(text.c_str());
            }
            Minecraft_setScreen(super->minecraft, NULL);
        }
        // Call Original Method
        original_keyPressed(super, key);
    };
    // Button Click
    vtable->buttonClicked = [](Screen *super, Button *button) {
        ChatScreen *self = (ChatScreen *) super;
        if (button == self->send) {
            // Send
            self->chat->setFocused(true);
            super->vtable->keyPressed(super, 0x0d);
        } else {
            // Call Original Method
            Screen_buttonClicked_non_virtual(super, button);
        }
    };
}
static Screen *create_chat_screen() {
    // Construct
    ChatScreen *screen = new ChatScreen;
    ALLOC_CHECK(screen);
    Screen_constructor(&screen->super.super);

    // Set VTable
    screen->super.super.vtable = get_chat_screen_vtable();

    // Return
    return (Screen *) screen;
}

// Init
void _init_chat_ui() {
    history = {};
    misc_run_on_game_key_press([](Minecraft *minecraft, int key) {
        if (key == 0x54) {
            if (Minecraft_isLevelGenerated(minecraft) && minecraft->screen == NULL) {
                Minecraft_setScreen(minecraft, create_chat_screen());
            }
            return true;
        } else {
            return false;
        }
    });
}

#endif
