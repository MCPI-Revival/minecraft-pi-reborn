#include "init.h"

__attribute__((constructor)) static void init() {
    init_compat();
    init_server();
    init_game_mode();
    init_input();
    init_misc();
    init_camera();
    init_options();
    init_textures();
}