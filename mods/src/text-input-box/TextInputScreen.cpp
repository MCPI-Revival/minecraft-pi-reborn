#include <libreborn/libreborn.h>

#include <mods/text-input-box/TextInputScreen.h>

// VTable
void TextInputScreen::setup(Screen_vtable *vtable) {
    static Screen_keyPressed_t original_keyPressed = vtable->keyPressed;
    vtable->keyPressed = [](Screen *super2, int key) {
        original_keyPressed(super2, key);
        TextInputScreen *self = (TextInputScreen *) super2;
        for (int i = 0; i < int(self->m_textInputs->size()); i++) {
            TextInputBox *textInput = (*self->m_textInputs)[i];
            textInput->keyPressed(key);
        }
    };
    static Screen_keyboardNewChar_t original_keyboardNewChar = vtable->keyboardNewChar;
    vtable->keyboardNewChar = [](Screen *super2, char key) {
        original_keyboardNewChar(super2, key);
        TextInputScreen *self = (TextInputScreen *) super2;
        for (int i = 0; i < int(self->m_textInputs->size()); i++) {
            TextInputBox *textInput = (*self->m_textInputs)[i];
            textInput->charPressed(key);
        }
    };
    static Screen_mouseClicked_t original_mouseClicked = vtable->mouseClicked;
    vtable->mouseClicked = [](Screen *super2, int x, int y, int param_1) {
        original_mouseClicked(super2, x, y, param_1);
        TextInputScreen *self = (TextInputScreen *) super2;
        for (int i = 0; i < int(self->m_textInputs->size()); i++) {
            TextInputBox *textInput = (*self->m_textInputs)[i];
            textInput->onClick(x, y);
        }
    };
    static Screen_render_t original_render = vtable->render;
    vtable->render = [](Screen *super2, int x, int y, float param_1) {
        original_render(super2, x, y, param_1);
        TextInputScreen *self = (TextInputScreen *) super2;
        for (int i = 0; i < int(self->m_textInputs->size()); i++) {
            TextInputBox *textInput = (*self->m_textInputs)[i];
            textInput->tick();
            textInput->render();
        }
    };
    static Screen_init_t original_init = vtable->init;
    vtable->init = [](Screen *super2) {
        original_init(super2);
        TextInputScreen *self = (TextInputScreen *) super2;
        self->m_textInputs = new std::vector<TextInputBox *>;
    };
    static Screen_removed_t original_removed = vtable->removed;
    vtable->removed = [](Screen *super2) {
        original_removed(super2);
        TextInputScreen *self = (TextInputScreen *) super2;
        delete self->m_textInputs;
    };
}

