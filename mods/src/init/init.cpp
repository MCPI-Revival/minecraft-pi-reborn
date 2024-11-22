#include <libreborn/libreborn.h>
#include <mods/init/init.h>
#include <media-layer/core.h>
#include <symbols/minecraft.h>

__attribute__((constructor)) static void init() {
    reborn_init_patch();
    thunk_enabler = reborn_thunk_enabler;
    init_version();
    init_compat();
    if (reborn_is_server()) {
        init_server();
    }
    init_multiplayer();
    if (!reborn_is_headless()) {
        init_sound();
        init_shading();
    }
    init_input();
    init_sign();
    init_camera();
    if (!reborn_is_headless()) {
        init_atlas();
    }
    init_title_screen();
    if (!reborn_is_headless()) {
        init_skin();
    }
    init_fps();
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
    init_override();
    if (!reborn_is_server()) {
        init_benchmark();
    }
    if (!reborn_is_headless()) {
        init_screenshot();
        init_f3();
        init_multidraw();
        init_classic_ui();
    }
}
