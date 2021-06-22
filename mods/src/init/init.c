#include "init.h"

__attribute__((constructor)) static void init() {
    run_tests();
    init_compat();
#ifdef MCPI_SERVER_MODE
    init_server();
#endif
    init_game_mode();
    init_input();
    init_misc();
    init_death();
    init_camera();
    init_options();
    init_touch();
    init_textures();
    init_chat();
    init_home();
}
