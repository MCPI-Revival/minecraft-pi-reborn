#pragma once

#include <symbols/Screen.h>

#include <mods/text-input-box/TextInputBox.h>

struct TextInputScreen : CustomScreen {
    std::vector<TextInputBox *> *m_textInputs = nullptr;

    void keyPressed(int key) override;
    void keyboardNewChar(char key) override;
    void mouseClicked(int x, int y, int param_1) override;
    void render(int x, int y, float param_1) override;
    void init() override;
    ~TextInputScreen() override;
};
