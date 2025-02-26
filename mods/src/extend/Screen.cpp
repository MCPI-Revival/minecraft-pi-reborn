#include "internal.h"

// Easily Create Custom Screens
void CustomScreen::init() {
    Screen_vtable::base->init(self);
}
void CustomScreen::render(const int x, const int y, const float param_1) {
    Screen_vtable::base->render(self, x, y, param_1);
}
void CustomScreen::setupPositions() {
    Screen_vtable::base->setupPositions(self);
}
bool CustomScreen::handleBackEvent(const bool do_nothing) {
    return Screen_vtable::base->handleBackEvent(self, do_nothing);
}
void CustomScreen::tick() {
    Screen_vtable::base->tick(self);
}
void CustomScreen::buttonClicked(Button *button) {
    Screen_vtable::base->buttonClicked(self, button);
}
void CustomScreen::mouseClicked(const int x, const int y, const int param_1) {
    Screen_vtable::base->mouseClicked(self, x, y, param_1);
}
void CustomScreen::mouseReleased(const int x, const int y, const int param_1) {
    Screen_vtable::base->mouseReleased(self, x, y, param_1);
}
void CustomScreen::keyPressed(const int key) {
    Screen_vtable::base->keyPressed(self, key);
}
void CustomScreen::keyboardNewChar(const char key) {
    Screen_vtable::base->keyboardNewChar(self, key);
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