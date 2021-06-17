#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MCPI_SERVER_MODE
char *misc_get_launch_directory();
#endif

__attribute__((visibility("internal"))) void _init_misc_cpp();

#ifdef __cplusplus
}
#endif