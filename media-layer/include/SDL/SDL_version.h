#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct SDL_version {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} SDL_version;

#ifdef __cplusplus
}
#endif
