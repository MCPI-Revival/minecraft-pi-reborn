#include <vector>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "../init/init.h"
#include "../feature/feature.h"
#include "../input/input.h"
#include "sign.h"

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

#define BACKSPACE_KEY 8

static int is_valid_key(char key) {
    return (key >= 32 && key <= 126) || key == BACKSPACE_KEY;
}

// Store Text Input
std::vector<char> input;
void sign_key_press(char key) {
    if (is_valid_key(key)) {
        input.push_back(key);
    }
}
static void clear_input(__attribute__((unused)) unsigned char *minecraft) {
    input.clear();
}

// Handle Text Input
static void TextEditScreen_updateEvents_injection(unsigned char *screen) {
    // Call Original Method
    (*Screen_updateEvents)(screen);

    if (*(char *)(screen + 4) == '\0') {
        unsigned char *vtable = *(unsigned char **) screen;
        for (char key : input) {
            if (key == BACKSPACE_KEY) {
                // Handle Backspace
                (*(Screen_keyPressed_t *) (vtable + Screen_keyPressed_vtable_offset))(screen, BACKSPACE_KEY);
            } else {
                // Handle Normal Key
                (*(Screen_keyboardNewChar_t *) (vtable + Screen_keyboardNewChar_vtable_offset))(screen, key);
            }
        }
    }
    clear_input(NULL);
}

// Init
void init_sign() {
    if (feature_has("Fix Sign Placement", server_disabled)) {
        // Fix Signs
        patch_address(LocalPlayer_openTextEdit_vtable_addr, (void *) LocalPlayer_openTextEdit_injection);
        patch_address(TextEditScreen_updateEvents_vtable_addr, (void *) TextEditScreen_updateEvents_injection);
        // Clear input On Input Tick
        input_run_on_tick(clear_input);
    }
}
