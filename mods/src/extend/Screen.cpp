#include "internal.h"

// Easily Create Custom Screens
void CustomScreen::init() {
    _VTable::base->init(self);
}
void CustomScreen::render(const int x, const int y, const float param_1) {
    _VTable::base->render(self, x, y, param_1);
}
void CustomScreen::setupPositions() {
    _VTable::base->setupPositions(self);
}
bool CustomScreen::handleBackEvent(const bool do_nothing) {
    return _VTable::base->handleBackEvent(self, do_nothing);
}
void CustomScreen::tick() {
    _VTable::base->tick(self);
}
void CustomScreen::buttonClicked(Button *button) {
    _VTable::base->buttonClicked(self, button);
}
void CustomScreen::mouseClicked(const int x, const int y, const int param_1) {
    _VTable::base->mouseClicked(self, x, y, param_1);
}
void CustomScreen::mouseReleased(const int x, const int y, const int param_1) {
    _VTable::base->mouseReleased(self, x, y, param_1);
}
void CustomScreen::keyPressed(const int key) {
    _VTable::base->keyPressed(self, key);
}
void CustomScreen::keyboardNewChar(const char key) {
    _VTable::base->keyboardNewChar(self, key);
}

// VTable
SETUP_VTABLE(Screen)
    PATCH_VTABLE(init);
    PATCH_VTABLE(render);
    PATCH_VTABLE(setupPositions);
    PATCH_VTABLE(handleBackEvent);
    PATCH_VTABLE(tick);
    PATCH_VTABLE(buttonClicked);
    PATCH_VTABLE(mouseClicked);
    PATCH_VTABLE(mouseReleased);
    PATCH_VTABLE(keyPressed);
    PATCH_VTABLE(keyboardNewChar);
}