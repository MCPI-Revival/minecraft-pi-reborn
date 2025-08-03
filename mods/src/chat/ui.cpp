#include <symbols/minecraft.h>

#include <mods/text-input-box/TextInputScreen.h>
#include <mods/misc/misc.h>
#include <mods/touch/touch.h>
#include <mods/input/input.h>

#include "internal.h"

// Store History
static std::vector<std::string> &get_history() {
    static std::vector<std::string> history = {};
    return history;
}
void _chat_clear_history() {
    get_history().clear();
}

// Structure
struct ChatScreen final : TextInputScreen {
    TextInputBox *chat = nullptr;
    Button *send = nullptr;
    int history_pos = 0;
    const std::string starting_text;

    // Construction
    explicit ChatScreen(const std::string &start_text):
        TextInputScreen(),
        starting_text(start_text) {}

    // Init
    std::vector<std::string> local_history = {};
    void init() override {
        TextInputScreen::init();

        // Text Input
        chat = new TextInputBox;
        m_textInputs->push_back(chat);
        chat->init(self->font);
        chat->setFocused(true);
        chat->setText(starting_text);
        history_pos = get_history().size();
        local_history = get_history();
        local_history.push_back("");

        // Determine Max Length
        const std::string prefix = _chat_get_prefix(Strings::default_username);
        const int max_length = MAX_CHAT_MESSAGE_LENGTH - prefix.length();
        chat->setMaxLength(max_length);

        // Send Button
        send = touch_create_button(1, "Send");
        self->rendered_buttons.push_back(send);
        self->selectable_buttons.push_back(send);

        // Hide Chat Messages
        is_in_chat = true;
    }

    // Removal
    ~ChatScreen() override {
        is_in_chat = false;
        delete chat;
        send->destructor_deleting();
    }

    // Rendering
    void render(const int x, const int y, const float param_1) override {
        // Background
        self->renderBackground();
        // Render Chat
        self->minecraft->gui.renderChatMessages(self->height, 20, true, self->font);
        // Call Original Method
        TextInputScreen::render(x, y, param_1);
    }

    // Positioning
    void setupPositions() override {
        TextInputScreen::setupPositions();
        send->height = 24;
        send->width = 40;
        constexpr int x = 0;
        const int y = self->height - send->height;
        const int width = self->width - send->width;
        chat->setSize(x, y, width, send->height);
        send->y = self->height - send->height;
        send->x = x + width;
    }

    // Key Presses
    void keyPressed(const int key) override {
        if (chat->isFocused()) {
            if (key == MC_KEY_RETURN) {
                if (chat->getText().length() > 0) {
                    const std::string text = chat->getText();
                    if (get_history().size() == 0 || text != get_history().back()) {
                        get_history().push_back(text);
                    }
                    _chat_send_message_to_server(self->minecraft, text.c_str());
                }
                self->minecraft->setScreen(nullptr);
            } else if (key == MC_KEY_UP) {
                // Up
                local_history.at(history_pos) = chat->getText();
                // Change
                history_pos -= 1;
                if (history_pos < 0) history_pos = local_history.size() - 1;
                chat->setText(local_history.at(history_pos));
                return;
            } else if (key == MC_KEY_DOWN) {
                // Down
                local_history.at(history_pos) = chat->getText();
                // Change
                history_pos += 1;
                if (history_pos > int(local_history.size()) - 1) history_pos = 0;
                chat->setText(local_history.at(history_pos));
                return;
            }
        }
        // Call Original Method
        TextInputScreen::keyPressed(key);
    }

    // Button Click
    void buttonClicked(Button *button) override {
        if (button == send) {
            // Send
            chat->setFocused(true);
            self->keyPressed(0x0d);
        } else {
            // Call Original Method
            TextInputScreen::buttonClicked(button);
        }
    }
};
static Screen *create_chat_screen(const std::string &starting_text) {
    return (new ChatScreen(starting_text))->self;
}

// Init
void _init_chat_ui() {
    misc_run_on_game_key_press([](Minecraft *minecraft, const int key) {
        if (minecraft->isLevelGenerated() && minecraft->screen == nullptr) {
            if (key == MC_KEY_t) {
                minecraft->setScreen(create_chat_screen(""));
            } else if (key == MC_KEY_SLASH) {
                minecraft->setScreen(create_chat_screen("/"));
            } else {
                return false;
            }
        }
        return true;
    });
}
