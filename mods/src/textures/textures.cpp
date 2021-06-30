#include <libreborn/libreborn.h>
#include <libreborn/minecraft.h>

#include "../feature/feature.h"
#include "../init/init.h"

// Animated Water
static void Minecraft_tick_injection(unsigned char *minecraft, int32_t param_1, int32_t param_2) {
    // Call Original Method
    (*Minecraft_tick)(minecraft, param_1, param_2);

    // Tick Dynamic Textures
    unsigned char *textures = *(unsigned char **) (minecraft + Minecraft_textures_property_offset);
    if (textures != NULL) {
        (*Textures_tick)(textures, true);
    }
}

// Init
void init_textures() {
    // Tick Dynamic Textures (Animated Water)
    if (feature_has("Animated Water")) {
        overwrite_calls((void *) Minecraft_tick, (void *) Minecraft_tick_injection);
    }
}
