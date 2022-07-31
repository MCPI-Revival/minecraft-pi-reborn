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
        return (*sdl_key_to_minecraft_key)(sdl_key);
    }
}

// Open Sign Screen
static void LocalPlayer_openTextEdit_injection(unsigned char *local_player, unsigned char *sign) {
    if (*(int32_t *) (sign + TileEntity_id_property_offset) == 4) {
        unsigned char *minecraft = *(unsigned char **) (local_player + LocalPlayer_minecraft_property_offset);
        unsigned char *screen = (unsigned char *) ::operator new(TEXT_EDIT_SCREEN_SIZE);
        ALLOC_CHECK(screen);
        screen = (*TextEditScreen)(screen, sign);
        (*Minecraft_setScreen)(minecraft, screen);
    }
}

// Store Text Input
std::vector<char> input;
void sign_key_press(char key) {
    input.push_back(key);
}
static void clear_input(__attribute__((unused)) unsigned char *minecraft) {
    input.clear();
}

// Handle Text Input
static void TextEditScreen_updateEvents_injection(unsigned char *screen) {
    // Call Original Method
    (*Screen_updateEvents)(screen);

    if (!*(bool *)(screen + Screen_passthrough_input_property_offset)) {
        unsigned char *vtable = *(unsigned char **) screen;
        for (char key : input) {
            // Handle Normal Key
            (*(Screen_keyboardNewChar_t *) (vtable + Screen_keyboardNewChar_vtable_offset))(screen, key);
        }
    }
    clear_input(NULL);
}

// Init
void init_sign() {
    if (feature_has("Fix Sign Placement", server_disabled)) {
        // Handle Backspace
        overwrite_calls((void *) sdl_key_to_minecraft_key, (void *) sdl_key_to_minecraft_key_injection);
        // Fix Signs
        patch_address(LocalPlayer_openTextEdit_vtable_addr, (void *) LocalPlayer_openTextEdit_injection);
        patch_address(TextEditScreen_updateEvents_vtable_addr, (void *) TextEditScreen_updateEvents_injection);
        // Clear Input On Input Tick
        input_run_on_tick(clear_input);
    }
}
