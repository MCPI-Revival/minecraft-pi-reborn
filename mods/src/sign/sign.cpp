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
        return 8;
    } else {
        // Call Original Method
        return (*Common_sdl_key_to_minecraft_key)(sdl_key);
    }
}

// Open Sign Screen
static void LocalPlayer_openTextEdit_injection(LocalPlayer *local_player, TileEntity *sign) {
    if (sign->id == 4) {
        Minecraft *minecraft = local_player->minecraft;
        TextEditScreen *screen = alloc_TextEditScreen();
        ALLOC_CHECK(screen);
        screen = (*TextEditScreen_constructor)(screen, (SignTileEntity *) sign);
        (*Minecraft_setScreen)(minecraft, (Screen *) screen);
    }
}

// Store Text Input
std::vector<char> input;
void sign_key_press(char key) {
    input.push_back(key);
}
static void clear_input(__attribute__((unused)) Minecraft *minecraft) {
    input.clear();
}

// Handle Text Input
static void TextEditScreen_updateEvents_injection(TextEditScreen *screen) {
    // Call Original Method
    (*TextEditScreen_updateEvents_non_virtual)(screen);

    if (!screen->passthrough_input) {
        for (char key : input) {
            // Handle Normal Key
            screen->vtable->keyboardNewChar(screen, key);
        }
    }
    clear_input(NULL);
}

// Init
void init_sign() {
    if (feature_has("Fix Sign Placement", server_disabled)) {
        // Handle Backspace
        overwrite_calls((void *) Common_sdl_key_to_minecraft_key, (void *) sdl_key_to_minecraft_key_injection);
        // Fix Signs
        patch_address(LocalPlayer_openTextEdit_vtable_addr, (void *) LocalPlayer_openTextEdit_injection);
        patch_address(TextEditScreen_updateEvents_vtable_addr, (void *) TextEditScreen_updateEvents_injection);
        // Clear Input On Input Tick
        input_run_on_tick(clear_input);
    }
}
