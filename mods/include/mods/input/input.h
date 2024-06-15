#pragma once

#include <symbols/minecraft.h>

extern "C" {
typedef void (*input_tick_function_t)(Minecraft *minecraft);
void input_run_on_tick(input_tick_function_t function);

void input_set_is_right_click(int val);
int input_back();
void input_drop(int drop_slot);

void input_set_is_left_click(int val);

void input_set_mouse_grab_state(int state);
}