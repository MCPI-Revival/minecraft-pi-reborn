#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void input_set_is_right_click(int val);
void input_hide_gui();
void input_third_person();
void input_back();

void input_set_is_left_click(int val);

void input_set_mouse_grab_state(int state);

#ifdef __cplusplus
}
#endif
