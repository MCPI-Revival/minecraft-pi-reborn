#pragma once

struct Button;

MCPI_MODS_PUBLIC extern int touch_gui;
extern "C" {
MCPI_MODS_PUBLIC Button *touch_create_button(int id, const std::string &text);
}