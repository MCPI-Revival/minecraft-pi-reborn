#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MCPI_SERVER_MODE
char *home_get_launch_directory();
#endif

char *home_get(); // Remember To free()

#ifdef __cplusplus
}
#endif
