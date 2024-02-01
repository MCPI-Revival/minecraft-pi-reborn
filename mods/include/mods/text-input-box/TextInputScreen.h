#pragma once

#include <symbols/minecraft.h>

#include <mods/text-input-box/TextInputBox.h>

struct TextInputScreen {
    Screen super;
    std::vector<TextInputBox *> *m_textInputs;

    static void setup(Screen_vtable *vtable);
};
