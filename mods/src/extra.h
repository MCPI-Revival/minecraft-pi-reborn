#ifndef EXTRA_H

#define EXTRA_H

#ifdef __cplusplus
extern "C" {
#endif

int has_feature(const char *name);

void key_press(char key);
void clear_input();

#ifdef __cplusplus
}
#endif

#endif
