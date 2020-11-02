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

    __attribute((constructor)) static void init() {
        // Implement AppPlatform::readAssetFile So Translations Work
        overwrite((void *) 0x12b10, (void *) readAssetFile);

        if (extra_has_feature("Fix Sign Placement")) {
            // Fix Signs
            patch_address((void *) 0x106460, (void *) openTextEdit);
            Screen_updateEvents_original = overwrite((void *) Screen_updateEvents, (void *) Screen_updateEvents_injection);
        }
    }
}
