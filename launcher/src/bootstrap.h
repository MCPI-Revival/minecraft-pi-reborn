#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void pre_bootstrap(int argc, char *argv[]);
void bootstrap(int argc, char *argv[]);
void copy_sdk(char *binary_directory, int log_with_debug);
void bootstrap_mods(char *binary_directory);

#ifdef __cplusplus
}
#endif
