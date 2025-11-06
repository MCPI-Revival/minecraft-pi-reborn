#include <mods/init/init.h>
#include <mods/feature/feature.h>

#include "internal.h"

// Init
void init_display_lists() {
    if (feature_has("Use OpenGL Display Lists", server_disabled)) {
        _init_display_lists_chunks();
        _init_display_lists_font();
    }
}