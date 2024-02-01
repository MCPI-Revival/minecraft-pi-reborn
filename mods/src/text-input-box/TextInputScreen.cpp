#include <libreborn/libreborn.h>

#include <mods/text-input-box/TextInputScreen.h>

// VTable
void TextInputScreen::setup(Screen_vtable *vtable) {
    vtable->keyPressed = [](Screen *super2, int key) {
        Screen_keyPressed_non_virtual(super2, key);
        TextInputScreen *self = (TextInputScreen *) super2;
        for (int i = 0; i < int(self->m_textInputs.size()); i++) {
            TextInputBox *textInput = self->m_textInputs[i];
            textInput->keyPressed(key);
        }
    };
    vtable->keyboardNewChar = [](Screen *super2, char key) {
        Screen_keyboardNewChar_non_virtual(super2, key);
        TextInputScreen *self = (TextInputScreen *) super2;
        for (int i = 0; i < int(self->m_textInputs.size()); i++) {
            TextInputBox *textInput = self->m_textInputs[i];
            textInput->charPressed(key);
        }
    };
    vtable->mouseClicked = [](Screen *super2, int x, int y, int param_1) {
        Screen_mouseClicked_non_virtual(super2, x, y, param_1);
        TextInputScreen *self = (TextInputScreen *) super2;
        for (int i = 0; i < int(self->m_textInputs.size()); i++) {
            TextInputBox *textInput = self->m_textInputs[i];
            textInput->onClick(x, y);
        }
    };
    vtable->render = [](Screen *super2, int x, int y, float param_1) {
        Screen_render_non_virtual(super2, x, y, param_1);
        TextInputScreen *self = (TextInputScreen *) super2;
        for (int i = 0; i < int(self->m_textInputs.size()); i++) {
            TextInputBox *textInput = self->m_textInputs[i];
            textInput->tick();
            textInput->render();
        }
    };
}
