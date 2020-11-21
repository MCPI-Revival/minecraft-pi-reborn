#ifndef EXTRA_H

#define EXTRA_H

#ifdef __cplusplus
extern "C" {
#endif

int extra_has_feature(const char *name);
int extra_get_mode();

void extra_key_press(char key);
void extra_clear_input();

void extra_set_is_right_click(int val);
void extra_hide_gui();
void extra_third_person();

int extra_get_smooth_lighting();

#ifdef __cplusplus
}
#endif

#endif
