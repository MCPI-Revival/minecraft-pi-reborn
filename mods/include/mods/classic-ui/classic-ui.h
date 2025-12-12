#pragma once

#include <functional>

struct Minecraft;
struct GuiComponent;

const std::function<void(GuiComponent *, int, int, int, int, int, int, int, int)> get_blit_with_classic_hud_offset();
extern "C" {
int get_classic_hud_y_offset(Minecraft *minecraft);
}