#include <vector>

#include <SDL/SDL.h>
#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include <mods/input/input.h>
#include <mods/sign/sign.h>

// Handle Backspace
static int32_t sdl_key_to_minecraft_key_injection(int32_t sdl_key) {
    if (sdl_key == SDLK_BACKSPACE) {
        return 0x8;
    } else if (sdl_key == SDLK_DELETE) {
        return 0x2e;
    } else if (sdl_key == SDLK_LEFT) {
        return 0x25;
    } else if (sdl_key == SDLK_RIGHT) {
        return 0x27;
    } else {
        // Call Original Method
        return Common_sdl_key_to_minecraft_key(sdl_key);
    }
}

// Open Sign Screen
static void LocalPlayer_openTextEdit_injection(LocalPlayer *local_player, TileEntity *sign) {
    if (sign->id == 4) {
        Minecraft *minecraft = local_player->minecraft;
        TextEditScreen *screen = alloc_TextEditScreen();
        ALLOC_CHECK(screen);
        screen = TextEditScreen_constructor(screen, (SignTileEntity *) sign);
        Minecraft_setScreen(minecraft, (Screen *) screen);
    }
}

// Store Text Input
void sign_key_press(char key) {
    Keyboard__inputText.push_back(key);
}

// Init
void init_sign() {
    if (feature_has("Fix Sign Placement", server_disabled)) {
        // Fix Signs
        patch_address(LocalPlayer_openTextEdit_vtable_addr, (void *) LocalPlayer_openTextEdit_injection);
    }

    // Handle Backspace
    overwrite_calls((void *) Common_sdl_key_to_minecraft_key, (void *) sdl_key_to_minecraft_key_injection);
}
