#include <string>
#include <vector>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include "skin-internal.h"

// Base64-URL Encode/Decode (https://stackoverflow.com/a/57314480)
static constexpr char base64_url_alphabet[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'
};
static std::string base64_encode(const std::string &data) {
    std::string out;
    int val = 0;
    int valb = -6;
    const size_t len = data.length();
    for (unsigned int i = 0; i < len; i++) {
        const unsigned char c = data[i];
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_url_alphabet[(val >> valb) & 0x3f]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        out.push_back(base64_url_alphabet[((val << 8) >> (valb + 8)) & 0x3f]);
    }
    return out;
}

// Change Texture For Player Entities
static void Player_username_assign_injection(std::string *target, std::string *username) {
    // Call Original Method
    *target = *username;

    // Get Player
    unsigned char *player = ((unsigned char *) target) - Player_username_property_offset;
    // Get Texture
    std::string *texture = (std::string *) (player + Mob_texture_property_offset);

    // Set Texture
    *texture = '$' + base64_encode(*username);
}
static void Player_username_assign_injection_2(std::string *target, const char *username) {
    std::string username_str = username;
    Player_username_assign_injection(target, &username_str);
}

// Change Texture For HUD
static int32_t Textures_loadAndBindTexture_injection(unsigned char *textures, __attribute__((unused)) std::string const& name) {
    // Change Texture
    static std::string new_texture;
    if (new_texture.length() == 0) {
        std::string username = base64_encode(*default_username);
        new_texture = '$' + username;
    }

    // Call Original Method
    return (*Textures_loadAndBindTexture)(textures, new_texture);
}

// Init
void init_skin() {
    // Check Feature Flag
    if (feature_has("Load Custom Skins", server_disabled)) {
        // LocalPlayer
        overwrite_call((void *) 0x44c28, (void *) Player_username_assign_injection);
        // RemotePlayer
        overwrite_call((void *) 0x6ce58, (void *) Player_username_assign_injection_2);
        // ServerPlayer
        overwrite_call((void *) 0x7639c, (void *) Player_username_assign_injection_2);

        // HUD
        overwrite_call((void *) 0x4c6d0, (void *) Textures_loadAndBindTexture_injection);

        // Loader
        _init_skin_loader();
    }
}
