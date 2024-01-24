#include <libreborn/libreborn.h>
#include <mods/init/init.h>
#include <media-layer/core.h>
#include <symbols/minecraft.h>

__attribute__((constructor)) static void init(int argc, char *argv[]) {
    media_ensure_loaded();
    run_tests();
    init_symbols();
    init_version();
    init_compat();
#ifdef MCPI_SERVER_MODE
    init_server();
#else
    init_multiplayer();
#endif
#ifndef MCPI_HEADLESS_MODE
    init_sound();
    init_input();
    init_sign();
    init_camera();
    init_touch();
    init_atlas();
    init_title_screen();
    init_skin();
#endif
    init_textures();
    init_creative();
    init_game_mode();
    init_misc();
    init_death();
    init_options();
    init_chat();
    init_bucket();
    init_home();
#ifndef MCPI_SERVER_MODE
    init_benchmark(argc, argv);
#else
    (void) argc;
    (void) argv;
#endif
}
