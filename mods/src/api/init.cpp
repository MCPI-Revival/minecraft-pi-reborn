#include "internal.h"

#include <mods/feature/feature.h>
#include <mods/init/init.h>

// Init
void init_api() {
    if (feature_has("Implement RaspberryJuice API", server_enabled)) {
        _init_api_commands();
    }
    // Miscellaneous Changes
    _init_api_socket();
    _init_api_misc();
}
