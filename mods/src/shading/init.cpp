#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include <mods/tesselator/tesselator.h>

#include "internal.h"

// Init
void init_shading() {
    if (feature_has("Proper Entity Shading", server_disabled)) {
        advanced_tesselator_enable();
        _init_normals();
        _init_lighting();
    }
}