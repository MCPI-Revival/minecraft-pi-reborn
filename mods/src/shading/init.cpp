#include <mods/init/init.h>
#include <mods/feature/feature.h>

#include "internal.h"

// Init
void init_shading() {
    if (feature_has("Proper Entity Shading", server_disabled)) {
        _init_custom_tesselator();
        _init_normals();
        _init_lighting();
    }
}