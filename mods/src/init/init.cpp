#include <libreborn/patch.h>
#include <libreborn/config.h>

#include <mods/init/init.h>
#include <symbols/minecraft.h>

// Init
__attribute__((constructor)) static void init() {
    if (reborn_is_server()) {
        init_server_flags();
    }
    reborn_init_patch();
    thunk_enabler = reborn_thunk_enabler;
    init_version();
    init_compat();
    if (reborn_is_server()) {
        init_server();
    }
    init_multiplayer();
    if (!reborn_is_headless()) {
        init_display_lists();
        init_sound();
        init_shading();
    }
    init_input();
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
    init_api();
    if (!reborn_is_server()) {
        init_benchmark();
    }
    if (!reborn_is_headless()) {
        init_screenshot();
        init_f3();
        init_classic_ui();
    }
}

// Instantiate Some Templates To Make Sure Everything Works
typedef std::remove_pointer_t<decltype(Minecraft_init)> func_t;
template void overwrite_call<func_t>(void *, func_t *, func_t::ptr_type, bool);
template void overwrite_calls<func_t>(func_t *, func_t::overwrite_type);
template void overwrite_calls_within<func_t>(void *, void *, func_t *, func_t::ptr_type);
template void patch_vtable<func_t>(const func_t *, func_t::ptr_type);