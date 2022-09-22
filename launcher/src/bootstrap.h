#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void run_simple_command(const char *const command[], const char *error);

void pre_bootstrap(int argc, char *argv[]);
void bootstrap(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif
