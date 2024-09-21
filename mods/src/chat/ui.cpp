#include "chat-internal.h"

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/chat/chat.h>
#include <mods/text-input-box/TextInputScreen.h>
#include <mods/misc/misc.h>
#include <mods/touch/touch.h>
#include <mods/input/input.h>

static std::vector<std::string> &get_history() {
    static std::vector<std::string> history = {};
    return history;
}

// Structure
EXTEND_STRUCT(ChatScreen, Screen, struct {
    TextInputScreen text_input;
    TextInputBox *chat;
    Button *send;
    int history_pos;
});
CUSTOM_VTABLE(chat_screen, Screen) {
    TextInputScreen::setup<ChatScreen>(vtable);
    // Init
    static std::vector<std::string> local_history = {};
    static Screen_init_t original_init = vtable->init;
    vtable->init = [](Screen *super) {
        original_init(super);
        ChatScreen *self = (ChatScreen *) super;
        // Text Input
        self->data.chat = new TextInputBox;
        self->data.text_input.m_textInputs->push_back(self->data.chat);
        self->data.chat->init(super->font);
        self->data.chat->setFocused(true);
        self->data.history_pos = get_history().size();
        local_history = get_history();
        local_history.push_back("");
        // Determine Max Length
        const std::string prefix = _chat_get_prefix(Strings::default_username);
        const int max_length = MAX_CHAT_MESSAGE_LENGTH - prefix.length();
        self->data.chat->setMaxLength(max_length);
        // Send Button
        self->data.send = touch_create_button(1, "Send");
        super->rendered_buttons.push_back(self->data.send);
        super->selectable_buttons.push_back(self->data.send);
        // Hide Chat Messages
        is_in_chat = true;
    };
    // Removal
    static Screen_removed_t original_removed = vtable->removed;
    vtable->removed = [](Screen *super) {
        original_removed(super);
        is_in_chat = false;
        const ChatScreen *self = (ChatScreen *) super;
        delete self->data.chat;
        self->data.send->destructor_deleting();
    };
    // Rendering
    static Screen_render_t original_render = vtable->render;
    vtable->render = [](Screen *super, const int x, const int y, const float param_1) {
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
        const ChatScreen *self = (ChatScreen *) super;
        self->data.send->height = 24;
        self->data.send->width = 40;
        constexpr int x = 0;
        const int y = super->height - self->data.send->height;
        const int width = super->width - self->data.send->width;
        self->data.chat->setSize(x, y, width, self->data.send->height);
        self->data.send->y = super->height - self->data.send->height;
        self->data.send->x = x + width;
    };
    // Key Presses
    static Screen_keyPressed_t original_keyPressed = vtable->keyPressed;
    vtable->keyPressed = [](Screen *super, const int key) {
        // Handle Enter
        ChatScreen *self = (ChatScreen *) super;
        if (self->data.chat->isFocused()) {
            if (key == MC_KEY_RETURN) {
                if (self->data.chat->getText().length() > 0) {
                    const std::string text = self->data.chat->getText();
                    if (get_history().size() == 0 || text != get_history().back()) {
                        get_history().push_back(text);
                    }
                    _chat_send_message(super->minecraft, text.c_str());
                }
                super->minecraft->setScreen(nullptr);
            } else if (key == MC_KEY_UP) {
                // Up
                local_history.at(self->data.history_pos) = self->data.chat->getText();
                // Change
                self->data.history_pos -= 1;
                if (self->data.history_pos < 0) self->data.history_pos = local_history.size() - 1;
                self->data.chat->setText(local_history.at(self->data.history_pos));
                return;
            } else if (key == MC_KEY_DOWN) {
                // Down
                local_history.at(self->data.history_pos) = self->data.chat->getText();
                // Change
                self->data.history_pos += 1;
                if (self->data.history_pos > int(local_history.size()) - 1) self->data.history_pos = 0;
                self->data.chat->setText(local_history.at(self->data.history_pos));
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
        if (button == self->data.send) {
            // Send
            self->data.chat->setFocused(true);
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
    screen->super()->constructor();

    // Set VTable
    screen->super()->vtable = get_chat_screen_vtable();

    // Return
    return (Screen *) screen;
}

// Init
void _init_chat_ui() {
    misc_run_on_game_key_press([](Minecraft *minecraft, const int key) {
        if (key == MC_KEY_t) {
            if (minecraft->isLevelGenerated() && minecraft->screen == nullptr) {
                minecraft->setScreen(create_chat_screen());
            }
            return true;
        } else {
            return false;
        }
    });
}
