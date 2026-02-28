#pragma once

#include <functional>

struct Minecraft;
struct GuiComponent;

MCPI_MODS_PUBLIC const std::function<void(GuiComponent *, int, int, int, int, int, int, int, int)> get_blit_with_classic_hud_offset();
extern "C" {
MCPI_MODS_PUBLIC int get_classic_hud_y_offset(Minecraft *minecraft);
}