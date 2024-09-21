#pragma once

#include <symbols/minecraft.h>

extern int touch_gui;
extern "C" {
Button *touch_create_button(int id, const std::string &text);
}