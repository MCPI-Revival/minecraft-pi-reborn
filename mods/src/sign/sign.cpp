#include <vector>

#include <libreborn/patch.h>

#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include <mods/sign/sign.h>

// Open Sign Screen
static void LocalPlayer_openTextEdit_injection(__attribute__((unused)) LocalPlayer_openTextEdit_t original, LocalPlayer *local_player, TileEntity *sign) {
    if (sign->type == 4) {
        Minecraft *minecraft = local_player->minecraft;
        TextEditScreen *screen = TextEditScreen::allocate();
        screen = screen->constructor((SignTileEntity *) sign);
        minecraft->setScreen((Screen *) screen);
    }
}

// Store Text Input
void sign_key_press(const char key) {
    Keyboard::_inputText.push_back(key);
}

// Init
void init_sign() {
    if (feature_has("Fix Sign Placement", server_disabled)) {
        // Fix Signs
        overwrite_calls(LocalPlayer_openTextEdit, LocalPlayer_openTextEdit_injection);
    }
}
