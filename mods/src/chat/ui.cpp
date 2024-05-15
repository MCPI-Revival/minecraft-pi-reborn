#include "chat-internal.h"
#include <libreborn/libreborn.h>
#include <mods/chat/chat.h>
#include <mods/text-input-box/TextInputScreen.h>
#include <mods/misc/misc.h>
#include <mods/touch/touch.h>

static std::vector<std::string> &get_history() {
    static std::vector<std::string> history = {};
    return history;
}

// Structure
struct ChatScreen {
    TextInputScreen super;
    TextInputBox *chat;
    Button *send;
    int history_pos;
};
CUSTOM_VTABLE(chat_screen, Screen) {
    TextInputScreen::setup(vtable);
    // Init
    static std::vector<std::string> local_history = {};
    static Screen_init_t original_init = vtable->init;
    vtable->init = [](Screen *super) {
        original_init(super);
        ChatScreen *self = (ChatScreen *) super;
        // Text Input
        self->chat = TextInputBox::create();
        self->super.m_textInputs->push_back(self->chat);
        self->chat->init(super->font);
        self->chat->setFocused(true);
        self->history_pos = get_history().size();
        local_history = get_history();
        local_history.push_back("");
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
        self->send->destructor_deleting();
    };
    // Rendering
    static Screen_render_t original_render = vtable->render;
    vtable->render = [](Screen *super, int x, int y, float param_1) {
        // Background
        super->renderBackground();
        // Render Chat
        super->minecraft->gui.renderChatMessages(super->height, 20, true, super->font);
        // Call Original Method
        original_render(super, x, y, param_1);
    };
    // Positioning
    static Screen_setupPositions_t original_setupPositions = vtable->setupPositions;
    vtable->setupPositions = [](Screen *super) {
        original_setupPositions(super);
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
        if (self->chat->isFocused()) {
            if (key == 0x0d) {
                if (self->chat->getText().length() > 0) {
                    std::string text = self->chat->getText();
                    if (get_history().size() == 0 || text != get_history().back()) {
                        get_history().push_back(text);
                    }
                    _chat_send_message(super->minecraft, text.c_str());
                }
                super->minecraft->setScreen(nullptr);
            } else if (key == 0x26) {
                // Up
                local_history.at(self->history_pos) = self->chat->getText();
                // Change
                self->history_pos -= 1;
                if (self->history_pos < 0) self->history_pos = local_history.size() - 1;
                self->chat->setText(local_history.at(self->history_pos));
                return;
            } else if (key == 0x28) {
                // Down
                local_history.at(self->history_pos) = self->chat->getText();
                // Change
                self->history_pos += 1;
                if (self->history_pos > int(local_history.size()) - 1) self->history_pos = 0;
                self->chat->setText(local_history.at(self->history_pos));
                return;
            }
        }
        // Call Original Method
        original_keyPressed(super, key);
    };
    // Button Click
    static Screen_buttonClicked_t original_buttonClicked = vtable->buttonClicked;
    vtable->buttonClicked = [](Screen *super, Button *button) {
        ChatScreen *self = (ChatScreen *) super;
        if (button == self->send) {
            // Send
            self->chat->setFocused(true);
            super->keyPressed(0x0d);
        } else {
            // Call Original Method
            original_buttonClicked(super, button);
        }
    };
}
static Screen *create_chat_screen() {
    // Construct
    ChatScreen *screen = new ChatScreen;
    ALLOC_CHECK(screen);
    screen->super.super.constructor();

    // Set VTable
    screen->super.super.vtable = get_chat_screen_vtable();

    // Return
    return (Screen *) screen;
}

// Init
void _init_chat_ui() {
    misc_run_on_game_key_press([](Minecraft *minecraft, int key) {
        if (key == 0x54) {
            if (minecraft->isLevelGenerated() && minecraft->screen == nullptr) {
                minecraft->setScreen(create_chat_screen());
            }
            return true;
        } else {
            return false;
        }
    });
}
