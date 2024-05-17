#include <vector>

#include <SDL/SDL.h>
#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include <mods/input/input.h>
#include <mods/sign/sign.h>

// Handle Backspace
static int32_t sdl_key_to_minecraft_key_injection(Common_sdl_key_to_minecraft_key_t original, int32_t sdl_key) {
    if (sdl_key == SDLK_BACKSPACE) {
        return 0x8;
    } else if (sdl_key == SDLK_DELETE) {
        return 0x2e;
    } else if (sdl_key == SDLK_LEFT) {
        return 0x25;
    } else if (sdl_key == SDLK_RIGHT) {
        return 0x27;
    } else if (sdl_key == SDLK_F1) {
        return 0x70;
    } else if (sdl_key == SDLK_F5) {
        return 0x74;
    } else {
        // Call Original Method
        return original(sdl_key);
    }
}

// Open Sign Screen
static void LocalPlayer_openTextEdit_injection(LocalPlayer *local_player, TileEntity *sign) {
    if (sign->type == 4) {
        Minecraft *minecraft = local_player->minecraft;
        TextEditScreen *screen = new TextEditScreen;
        ALLOC_CHECK(screen);
        screen = screen->constructor((SignTileEntity *) sign);
        minecraft->setScreen((Screen *) screen);
    }
}

// Store Text Input
void sign_key_press(char key) {
    Keyboard::_inputText.push_back(key);
}

// Init
void init_sign() {
    if (feature_has("Fix Sign Placement", server_disabled)) {
        // Fix Signs
        patch_vtable(LocalPlayer_openTextEdit, LocalPlayer_openTextEdit_injection);
    }

    // Handle Backspace
    overwrite_calls(Common_sdl_key_to_minecraft_key, sdl_key_to_minecraft_key_injection);
}
