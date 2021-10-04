#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*input_tick_function_t)(unsigned char *minecraft);
void input_run_on_tick(input_tick_function_t function);

void input_set_is_right_click(int val);
void input_hide_gui();
void input_third_person();
int input_back();
void input_drop(int drop_slot);

void input_set_is_left_click(int val);

void input_set_mouse_grab_state(int state);

__attribute__((visibility("internal"))) void _init_attack();
__attribute__((visibility("internal"))) void _init_bow();
__attribute__((visibility("internal"))) void _init_misc();
__attribute__((visibility("internal"))) void _init_toggle();
__attribute__((visibility("internal"))) void _init_drop();

#ifdef __cplusplus
}
#endif
