#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void set_and_print_env(const char *name, char *value);

void bootstrap(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif
