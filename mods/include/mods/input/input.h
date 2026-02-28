#pragma once

extern "C" {
MCPI_MODS_PUBLIC void input_set_is_right_click(int val);
MCPI_MODS_PUBLIC void input_set_is_ctrl(bool val);
MCPI_MODS_PUBLIC void input_set_scroll(int direction);

enum {
#define KEY(name, value) MC_KEY_##name = (value),
#include "key-list.h"
#undef KEY
};
}