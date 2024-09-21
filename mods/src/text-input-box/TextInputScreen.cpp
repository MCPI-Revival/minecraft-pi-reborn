#include <mods/text-input-box/TextInputScreen.h>

// VTable
void TextInputScreen::keyPressed(const int key) const {
    for (int i = 0; i < int(m_textInputs->size()); i++) {
        TextInputBox *textInput = (*m_textInputs)[i];
        textInput->keyPressed(key);
    }
}
void TextInputScreen::keyboardNewChar(const char key) const {
    for (int i = 0; i < int(m_textInputs->size()); i++) {
        TextInputBox *textInput = (*m_textInputs)[i];
        textInput->charPressed(key);
    }
}
void TextInputScreen::mouseClicked(const int x, const int y, __attribute__((unused)) int param_1) const {
    for (int i = 0; i < int(m_textInputs->size()); i++) {
        TextInputBox *textInput = (*m_textInputs)[i];
        textInput->onClick(x, y);
    }
}
void TextInputScreen::render(__attribute__((unused)) int x, __attribute__((unused)) int y, __attribute__((unused)) float param_1) const {
    for (int i = 0; i < int(m_textInputs->size()); i++) {
        TextInputBox *textInput = (*m_textInputs)[i];
        textInput->tick();
        textInput->render();
    }
}
void TextInputScreen::init() {
    m_textInputs = new std::vector<TextInputBox *>;
}
void TextInputScreen::removed() const {
    delete m_textInputs;
}
