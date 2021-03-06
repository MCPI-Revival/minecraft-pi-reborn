#include "init.h"

__attribute__((constructor)) static void init() {
    run_tests();
    init_compat();
#ifdef MCPI_SERVER_MODE
    init_server();
#else
    init_multiplayer();
#endif
    init_game_mode();
    init_input();
    init_sign();
    init_misc();
    init_death();
    init_camera();
    init_options();
    init_touch();
    init_textures();
    init_atlas();
    init_chat();
    init_home();
    init_version();
}
