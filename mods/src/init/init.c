#include "init.h"

__attribute__((constructor)) static void init() {
    run_tests();
    init_compat();
    init_server();
    init_game_mode();
    init_input();
    init_misc();
    init_camera();
    init_options();
    init_textures();
    init_chat();
}