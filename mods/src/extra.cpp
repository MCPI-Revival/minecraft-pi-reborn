#include <string>
#include <fstream>
#include <streambuf>
#include <vector>

#include <unistd.h>

#include <libcore/libcore.h>

#include "extra.h"
#include "cxx11_util.h"

#include <cstdio>

extern "C" {
    static cxx11_string readAssetFile(__attribute__((unused)) unsigned char *obj, std::string const& path) {
        std::string full_path("./data/");
        full_path.append(path);
        std::ifstream stream(full_path);
        std::string str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        return create_cxx11_string(str.c_str());
    }

    typedef unsigned char *(*TextEditScreen_t)(unsigned char *, unsigned char *);
    static TextEditScreen_t TextEditScreen = (TextEditScreen_t) 0x3a840;

    typedef void (*Minecraft_setScreen_t)(unsigned char *, unsigned char *);
    static Minecraft_setScreen_t Minecraft_setScreen = (Minecraft_setScreen_t) 0x15d6c;

    static void openTextEdit(unsigned char *local_player, unsigned char *sign) {
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

    std::vector<char> input;
    void extra_key_press(char key) {
        if (is_valid_key(key)) {
            input.push_back(key);
        }
    }
    void extra_clear_input() {
        input.clear();
    }

    typedef void (*Screen_updateEvents_t)(unsigned char *screen);
    static Screen_updateEvents_t Screen_updateEvents = (Screen_updateEvents_t) 0x28eb8;
    static void *Screen_updateEvents_original = NULL;

    typedef void (*Screen_keyboardNewChar_t)(unsigned char *screen, char key);
    typedef void (*Screen_keyPressed_t)(unsigned char *screen, int32_t key);

    static void Screen_updateEvents_injection(unsigned char *screen) {
        // Call Original
        revert_overwrite((void *) Screen_updateEvents, Screen_updateEvents_original);
        (*Screen_updateEvents)(screen);
        revert_overwrite((void *) Screen_updateEvents, Screen_updateEvents_original);

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

    typedef unsigned char *(*ItemInstance_t)(unsigned char *item_instance, unsigned char *item);
    static ItemInstance_t ItemInstance_item = (ItemInstance_t) 0x9992c;
    static ItemInstance_t ItemInstance_tile = (ItemInstance_t) 0x998e4;

    typedef int32_t (*FillingContainer_addItem_t)(unsigned char *filling_container, unsigned char *item_instance);
    static FillingContainer_addItem_t FillingContainer_addItem = (FillingContainer_addItem_t) 0x92aa0;
    static void *FillingContainer_addItem_original = NULL;

    static void inventory_add_item(unsigned char *inventory, unsigned char *item, bool is_tile) {
        unsigned char *item_instance = (unsigned char *) ::operator new(0xc);
        item_instance = (*(is_tile ? ItemInstance_tile : ItemInstance_item))(item_instance, item);
        (*FillingContainer_addItem)(inventory, item_instance);
    }

    // Items
    static unsigned char **item_flintAndSteel = (unsigned char **) 0x17ba70;
    static unsigned char **item_snowball = (unsigned char **) 0x17bbb0;
    static unsigned char **item_shears = (unsigned char **) 0x17bbf0;
    static unsigned char **item_sign = (unsigned char **) 0x17bba4;
    // Tiles
    static unsigned char **tile_water = (unsigned char **) 0x181b3c;
    static unsigned char **tile_lava = (unsigned char **) 0x181cc8;
    static unsigned char **tile_glowing_obsidian = (unsigned char **) 0x181dcc;
    static unsigned char **tile_invisible_bedrock = (unsigned char **) 0x181d94;

    static int32_t FillingContainer_addItem_injection(unsigned char *filling_container, unsigned char *item_instance) {
        // Call Original
        revert_overwrite((void *) FillingContainer_addItem, FillingContainer_addItem_original);
        int32_t ret = (*FillingContainer_addItem)(filling_container, item_instance);
        revert_overwrite((void *) FillingContainer_addItem, FillingContainer_addItem_original);

        // Add After Sign
        if (*(int32_t *) (item_instance + 0x4) == *(int32_t *) (*item_sign + 0x4)) {
            // Add Items
            inventory_add_item(filling_container, *item_flintAndSteel, false);
            inventory_add_item(filling_container, *item_snowball, false);
            inventory_add_item(filling_container, *item_shears, false);
            // Add Tiles
            inventory_add_item(filling_container, *tile_water, true);
            inventory_add_item(filling_container, *tile_lava, true);
            inventory_add_item(filling_container, *tile_glowing_obsidian, true);
            inventory_add_item(filling_container, *tile_invisible_bedrock, true);
        }

        return ret;
    }

    __attribute((constructor)) static void init() {
        // Implement AppPlatform::readAssetFile So Translations Work
        overwrite((void *) 0x12b10, (void *) readAssetFile);

        if (extra_has_feature("Fix Sign Placement")) {
            // Fix Signs
            patch_address((void *) 0x106460, (void *) openTextEdit);
            Screen_updateEvents_original = overwrite((void *) Screen_updateEvents, (void *) Screen_updateEvents_injection);
        }

        if (extra_has_feature("Expand Creative Inventory")) {
            // Add Extra Items To Creative Inventory
            FillingContainer_addItem_original = overwrite((void *) FillingContainer_addItem, (void *) FillingContainer_addItem_injection);
        }
    }
}
