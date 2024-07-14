#include <libreborn/libreborn.h>
#include <mods/init/init.h>
#include <media-layer/core.h>
#include <symbols/minecraft.h>

__attribute__((constructor)) static void init() {
    thunk_enabler = reborn_thunk_enabler;
    media_ensure_loaded();
    reborn_init_patch();
    run_tests();
    init_version();
    init_compat();
    if (reborn_is_server()) {
        init_server();
    } else {
        init_multiplayer();
    }
    if (!reborn_is_headless()) {
        init_sound();
    }
    init_input();
    init_sign();
    init_camera();
    init_atlas();
    init_title_screen();
    if (!reborn_is_headless()) {
        init_skin();
        init_fps();
    }
    init_touch();
    init_textures();
    init_creative();
    init_game_mode();
    init_misc();
    init_death();
    init_options();
    init_chat();
    init_bucket();
    init_cake();
    init_home();
    init_override();
    if (!reborn_is_server()) {
        init_benchmark();
    }
    if (!reborn_is_headless()) {
        init_screenshot();
        init_f3();
        init_multidraw();
    }
}
