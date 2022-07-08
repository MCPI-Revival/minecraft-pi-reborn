#pragma once

#include <stdio.h>
#include <stdlib.h>

// Logging
#define INFO(format, ...) { fprintf(stderr, "[INFO]: " format "\n", ##__VA_ARGS__); }
#define WARN(format, ...) { fprintf(stderr, "[WARN]: " format "\n", ##__VA_ARGS__); }
#define DEBUG(format, ...) { const char *debug = getenv("MCPI_DEBUG"); if (debug != NULL) { fprintf(stderr, "[DEBUG]: " format "\n", ##__VA_ARGS__); } }
#define ERR(format, ...) { fprintf(stderr, "[ERR]: (%s:%i): " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); exit(EXIT_FAILURE); }
#define IMPOSSIBLE() ERR("This Should Never Be Called")
