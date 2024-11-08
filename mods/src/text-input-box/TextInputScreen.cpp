#include <mods/text-input-box/TextInputScreen.h>

// VTable
void TextInputScreen::keyPressed(const int key) {
    CustomScreen::keyPressed(key);
    for (int i = 0; i < int(m_textInputs->size()); i++) {
        TextInputBox *textInput = (*m_textInputs)[i];
        textInput->keyPressed(key);
    }
}
void TextInputScreen::keyboardNewChar(const char key) {
    CustomScreen::keyboardNewChar(key);
    for (int i = 0; i < int(m_textInputs->size()); i++) {
        TextInputBox *textInput = (*m_textInputs)[i];
        textInput->charPressed(key);
    }
}
void TextInputScreen::mouseClicked(const int x, const int y, const int param_1) {
    CustomScreen::mouseClicked(x, y, param_1);
    for (int i = 0; i < int(m_textInputs->size()); i++) {
        TextInputBox *textInput = (*m_textInputs)[i];
        textInput->onClick(x, y);
    }
}
void TextInputScreen::render(const int x, const int y, const float param_1) {
    CustomScreen::render(x, y, param_1);
    for (int i = 0; i < int(m_textInputs->size()); i++) {
        TextInputBox *textInput = (*m_textInputs)[i];
        textInput->tick();
        textInput->render();
    }
}
void TextInputScreen::init() {
    CustomScreen::init();
    m_textInputs = new std::vector<TextInputBox *>;
}
TextInputScreen::~TextInputScreen() {
    delete m_textInputs;
}
