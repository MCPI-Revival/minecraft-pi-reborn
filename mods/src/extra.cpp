#include <string>
#include <fstream>
#include <streambuf>
#include <vector>

#include <unistd.h>

#include <libcore/libcore.h>

#include "extra.h"
#include "cxx11_util.h"

#include "minecraft.h"

#include <cstdio>

extern "C" {
    // Read Asset File
    static cxx11_string AppPlatform_readAssetFile_injection(__attribute__((unused)) unsigned char *app_platform, std::string const& path) {
        std::string full_path("./data/");
        full_path.append(path);
        std::ifstream stream(full_path);
        std::string str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        return create_cxx11_string(str.c_str());
    }

    // Open Sign Screen
    static void LocalPlayer_openTextEdit_injection(unsigned char *local_player, unsigned char *sign) {
        if (*(int *)(sign + 0x18) == 4) {
            unsigned char *minecraft = *(unsigned char **) (local_player + 0xc90);
            unsigned char *screen = (unsigned char *) ::operator new(0xd0);
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
    void extra_key_press(char key) {
        if (is_valid_key(key)) {
            input.push_back(key);
        }
    }
    void extra_clear_input() {
        input.clear();
    }

    // Handle Text Input
    static void TextEditScreen_updateEvents_injection(unsigned char *screen) {
        // Call Original Method
        (*Screen_updateEvents)(screen);

        if (*(char *)(screen + 4) == '\0') {
            uint32_t vtable = *((uint32_t *) screen);
            for (char key : input) {
                if (key == BACKSPACE_KEY) {
                    // Handle Backspace
                    (*(Screen_keyPressed_t *) (vtable + 0x6c))(screen, BACKSPACE_KEY);
                } else {
                    // Handle Nrmal Key
                    (*(Screen_keyboardNewChar_t *) (vtable + 0x70))(screen, key);
                }
            }
        }
        extra_clear_input();
    }

    static void inventory_add_item(unsigned char *inventory, unsigned char *item, bool is_tile) {
        unsigned char *item_instance = (unsigned char *) ::operator new(0xc);
        item_instance = (*(is_tile ? ItemInstance_tile : ItemInstance_item))(item_instance, item);
        (*FillingContainer_addItem)(inventory, item_instance);
    }

    static int32_t FillingContainer_addItem_injection(unsigned char *filling_container, unsigned char *item_instance) {
        // Call Original
        int32_t ret = (*FillingContainer_addItem)(filling_container, item_instance);

        // Add Items
        inventory_add_item(filling_container, *item_flintAndSteel, false);
        inventory_add_item(filling_container, *item_snowball, false);
        inventory_add_item(filling_container, *item_egg, false);
        inventory_add_item(filling_container, *item_shears, false);
        for (int i = 0; i < 15; i++) {
            unsigned char *item_instance = (unsigned char *) ::operator new(0xc);
            item_instance = (*ItemInstance_damage)(item_instance, *item_dye_powder, 1, i);
            (*FillingContainer_addItem)(filling_container, item_instance);
        }
        // Add Tiles
        inventory_add_item(filling_container, *tile_water, true);
        inventory_add_item(filling_container, *tile_lava, true);
        inventory_add_item(filling_container, *tile_calmWater, true);
        inventory_add_item(filling_container, *tile_calmLava, true);
        inventory_add_item(filling_container, *tile_glowingObsidian, true);
        inventory_add_item(filling_container, *tile_topSnow, true);
        inventory_add_item(filling_container, *tile_ice, true);
        inventory_add_item(filling_container, *tile_invisible_bedrock, true);

        return ret;
    }

    static void Minecraft_tick_injection(unsigned char *minecraft, int32_t param_1, int32_t param_2) {
        // Call Original Method
        (*Minecraft_tick)(minecraft, param_1, param_2);

        // Tick Dynamic Textures
        unsigned char *textures = *(unsigned char **) (minecraft + 0x164);
        if (textures != NULL) {
            (*Textures_tick)(textures, true);
        }
    }

    __attribute((constructor)) static void init() {
        // Implement AppPlatform::readAssetFile So Translations Work
        overwrite((void *) AppPlatform_readAssetFile, (void *) AppPlatform_readAssetFile_injection);

        if (extra_has_feature("Fix Sign Placement")) {
            // Fix Signs
            patch_address(LocalPlayer_openTextEdit_vtable_addr, (void *) LocalPlayer_openTextEdit_injection);
            patch_address(TextEditScreen_updateEvents_vtable_addr, (void *) TextEditScreen_updateEvents_injection);
        }

        if (extra_has_feature("Expand Creative Inventory")) {
            // Add Extra Items To Creative Inventory (Only Replace Specific Function Call)
            overwrite_call((void *) 0x8e0fc, (void *) FillingContainer_addItem_injection);
        }

        if (extra_has_feature("Animated Water")) {
            // Tick Dynamic Textures (Animated Water)
            overwrite_calls((void *) Minecraft_tick, (void *) Minecraft_tick_injection);
        }
    }
}
