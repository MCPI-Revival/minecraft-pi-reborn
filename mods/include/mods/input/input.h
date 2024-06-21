#pragma once

extern "C" {
void input_set_is_right_click(int val);
void input_set_is_ctrl(bool val);

enum {
#define KEY(name, value) MC_KEY_##name = (value),
#include "key-list.h"
#undef KEY
};
}