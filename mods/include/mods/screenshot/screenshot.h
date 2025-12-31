#pragma once

struct Gui;

extern "C" {
void screenshot_take(Gui *gui, const char *dir = "");
}