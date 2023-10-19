#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include "skin-internal.h"

// Base64 Encode (https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594)
static std::string base64_encode(const std::string data) {
    static constexpr char encoding_table[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', '+', '/'
    };

    size_t in_len = data.size();
    size_t out_len = 4 * ((in_len + 2) / 3);
    std::string ret(out_len, '\0');
    size_t i;
    char *p = const_cast<char*>(ret.c_str());

    for (i = 0; i < in_len - 2; i += 3) {
        *p++ = encoding_table[(data[i] >> 2) & 0x3f];
        *p++ = encoding_table[((data[i] & 0x3) << 4) | ((int) (data[i + 1] & 0xf0) >> 4)];
        *p++ = encoding_table[((data[i + 1] & 0xf) << 2) | ((int) (data[i + 2] & 0xc0) >> 6)];
        *p++ = encoding_table[data[i + 2] & 0x3f];
    }
    if (i < in_len) {
        *p++ = encoding_table[(data[i] >> 2) & 0x3f];
        if (i == (in_len - 1)) {
            *p++ = encoding_table[((data[i] & 0x3) << 4)];
            *p++ = '=';
        }
        else {
            *p++ = encoding_table[((data[i] & 0x3) << 4) | ((int) (data[i + 1] & 0xf0) >> 4)];
            *p++ = encoding_table[((data[i + 1] & 0xf) << 2)];
        }
        *p++ = '=';
    }

    return ret;
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
