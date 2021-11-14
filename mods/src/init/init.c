#include "init.h"

#include <media-layer/core.h>

__attribute__((constructor)) static void init() {
    media_ensure_loaded();
    run_tests();
    init_version();
    init_compat();
#ifdef MCPI_SERVER_MODE
    init_server();
#else
    init_multiplayer();
    init_sound();
    init_input();
    init_sign();
    init_creative();
    init_camera();
    init_touch();
    init_textures();
    init_atlas();
#endif
    init_game_mode();
    init_misc();
    init_death();
    init_options();
    init_chat();
    init_home();
#if !defined(MCPI_SERVER_MODE) && !defined(MCPI_HEADLESS_MODE)
    init_benchmark();
#endif
}
