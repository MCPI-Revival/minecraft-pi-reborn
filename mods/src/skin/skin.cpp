#include <cstdint>

#include <libreborn/patch.h>
#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include <mods/misc/misc.h>
#include "internal.h"

// Change Texture For Player Entities
static std::string get_skin_texture_path(const std::string &username) {
    return '$' + misc_base64_encode(username);
}
static void Player_username_assign_injection(std::string *target, const std::string &username) {
    // Call Original Method
    *target = username;

    // Get Player
    Player *player = (Player *) (((unsigned char *) target) - offsetof(Player, username));
    // Get Texture
    std::string *texture = &player->texture;

    // Set Texture
    *texture = get_skin_texture_path(username);
}
static void Player_username_assign_injection_2(std::string *target, const char *username) {
    const std::string username_str = username;
    Player_username_assign_injection(target, username_str);
}

// Change Texture For HUD
static uint32_t ItemInHandRenderer_render_Textures_loadAndBindTexture_injection(Textures *textures, __attribute__((unused)) std::string const& name) {
    // Change Texture
    static std::string new_texture;
    if (new_texture.length() == 0) {
        new_texture = get_skin_texture_path(Strings::default_username);
    }

    // Call Original Method
    return textures->loadAndBindTexture(new_texture);
}

// Init
void init_skin() {
    // Check Feature Flag
    if (feature_has("Load Custom Skins", server_disabled)) {
        // LocalPlayer
        overwrite_call_manual((void *) 0x44c28, (void *) Player_username_assign_injection);
        // RemotePlayer
        overwrite_call_manual((void *) 0x6ce58, (void *) Player_username_assign_injection_2);
        // ServerPlayer
        overwrite_call_manual((void *) 0x7639c, (void *) Player_username_assign_injection_2);

        // HUD
        overwrite_call((void *) 0x4c6d0, Textures_loadAndBindTexture, ItemInHandRenderer_render_Textures_loadAndBindTexture_injection);

        // Loader
        _init_skin_loader();
    }
}
