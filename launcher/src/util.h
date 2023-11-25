#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void run_simple_command(const char *const command[], const char *error);

void chop_last_component(char **str);
char *get_binary_directory();

#ifdef __cplusplus
}
#endif
