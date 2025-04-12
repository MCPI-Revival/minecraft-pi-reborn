#include "internal.h"

// Init
void _init_multiplayer_loading() {
    _init_multiplayer_loading_packets();
    _init_multiplayer_loading_thread();
    _init_multiplayer_loading_misc();
    _init_multiplayer_loading_terrain();
}