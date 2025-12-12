#include <mods/input/input.h>
#include <mods/feature/feature.h>

#include <symbols/Common.h>

#include <libreborn/patch.h>

#include <SDL/SDL.h>

#include "internal.h"

// Translator
static int32_t sdl_key_to_minecraft_key_injection(Common_sdl_key_to_minecraft_key_t original, const int32_t sdl_key) {
    switch (sdl_key) {
#define KEY(name, value) case SDLK_##name: return MC_KEY_##name;
#include <mods/input/key-list.h>
#undef KEY
        default: {
            // Call Original Method
            return original(sdl_key);
        }
    }
}

// Init
void _init_keys() {
    if (feature_has("Extend Supported Keycodes", server_disabled)) {
        overwrite_calls(Common_sdl_key_to_minecraft_key, sdl_key_to_minecraft_key_injection);
    }
}