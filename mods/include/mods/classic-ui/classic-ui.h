#pragma once

#include <symbols/minecraft.h>

extern "C" {
std::remove_reference_t<GuiComponent_blit_t> get_blit_with_classic_hud_offset();
int get_classic_hud_y_offset(Minecraft *minecraft);
}