#ifndef FEATURE_H

#define FEATURE_H

#ifdef __cplusplus
extern "C" {
#endif

int feature_has(const char *name);
int feature_get_mode();

#ifdef __cplusplus
}
#endif

#endif
