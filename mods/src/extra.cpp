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

    typedef void (*setScreen_t)(unsigned char *, unsigned char *);
    static setScreen_t setScreen = (setScreen_t) 0x15d6c;

    static void openTextEdit(unsigned char *local_player, unsigned char *sign) {
        if (*(int *)(sign + 0x18) == 4) {
            unsigned char *minecraft = *(unsigned char **) (local_player + 0xc90);
            unsigned char *screen = (unsigned char *) ::operator new(0xd0);
            screen = (*TextEditScreen)(screen, sign);
            (*setScreen)(minecraft, screen);
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

    typedef void (*updateEvents_t)(unsigned char *screen);
    static updateEvents_t updateEvents = (updateEvents_t) 0x28eb8;
    static void *updateEvents_original = NULL;

    typedef void (*keyboardNewChar_t)(unsigned char *screen, char key);
    typedef void (*keyPressed_t)(unsigned char *screen, int32_t key);

    static void updateEvents_injection(unsigned char *screen) {
        // Call Original
        revert_overwrite((void *) updateEvents, updateEvents_original);
        (*updateEvents)(screen);
        revert_overwrite((void *) updateEvents, updateEvents_original);

        if (*(char *)(screen + 4) == '\0') {
            uint32_t vtable = *((uint32_t *) screen);
            for (char key : input) {
                if (key == BACKSPACE_KEY) {
                    // Handle Backspace
                    (*(keyPressed_t *) (vtable + 0x6c))(screen, BACKSPACE_KEY);
                } else {
                    // Handle Nrmal Key
                    (*(keyboardNewChar_t *) (vtable + 0x70))(screen, key);
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
            updateEvents_original = overwrite((void *) updateEvents, (void *) updateEvents_injection);
        }
    }
}
