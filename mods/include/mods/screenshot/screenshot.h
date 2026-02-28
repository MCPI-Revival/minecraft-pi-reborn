#pragma once

struct Gui;

extern "C" {
MCPI_MODS_PUBLIC void screenshot_take(Gui *gui, const char *dir = "");
}