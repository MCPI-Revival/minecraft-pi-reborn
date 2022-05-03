#pragma once

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Colors
char *color_reset();
char *color_yellow();
char *color_faint();
char *color_red();

// Logging
#define INFO(format, ...) { fprintf(stderr, "[INFO]: " format "\n", ##__VA_ARGS__); }
#define WARN(format, ...) { fprintf(stderr, "%s[WARN]: " format "%s\n", color_yellow(), ##__VA_ARGS__, color_reset()); }
#define DEBUG(format, ...) { const char *debug = getenv("MCPI_DEBUG"); if (debug != NULL && strlen(debug) > 0) { fprintf(stderr, "%s[DEBUG]: " format "%s\n", color_faint(), ##__VA_ARGS__, color_reset()); } }
#define ERR(format, ...) { fprintf(stderr, "%s[ERR]: (%s:%i): " format "%s\n", color_red(), __FILE__, __LINE__, ##__VA_ARGS__, color_reset()); exit(EXIT_FAILURE); }
#define IMPOSSIBLE() ERR("This Should Never Be Called")

#ifdef __cplusplus
}
#endif
