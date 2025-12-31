#include <cstdlib>
#include <unistd.h>

#include "stb_image_write.h"

#include <libreborn/log.h>
#include <libreborn/util/util.h>
#include <libreborn/util/string.h>
#include <libreborn/config.h>

#include <GLES/gl.h>
#include <symbols/Minecraft.h>

#include <mods/screenshot/screenshot.h>
#include <mods/misc/misc.h>
#include <mods/input/input.h>
#include <mods/init/init.h>
#include <mods/feature/feature.h>

// Ensure Screenshots Folder Exists
static std::string get_screenshot_dir(const std::string &sub = "") {
    std::string dir = home_get();
    dir += "/screenshots";
    ensure_directory(dir.c_str());
    if (!sub.empty()) {
        dir += '/';
        dir += sub;
        ensure_directory(dir.c_str());
    }
    return dir;
}
static std::string get_screenshot(const std::string &filename, const std::string &dir) {
    return get_screenshot_dir(dir) + '/' + filename;
}

// Take Screenshot
static int save_png(const char *filename, const unsigned char *pixels, const int line_size, const int width, const int height) {
    // Setup
    stbi_flip_vertically_on_write(1);

    // Write Image
    return !stbi_write_png(filename, width, height, 4, pixels, line_size);
}
void screenshot_take(Gui *gui, const char *dir) {
    // Check
    if (reborn_is_headless()) {
        IMPOSSIBLE();
    }

    // Get Timestamp
    const std::string time = format_time("%Y-%m-%d_%H.%M.%S");

    // Prevent Overwriting Screenshots
    int num = 0;
    std::string filename;
    std::string file;
    do {
        // Get File Name
        filename = time;
        if (num > 0) {
            filename += '-' + safe_to_string(num);
        }
        filename += ".png";
        // Get Full Path
        file = get_screenshot(filename, dir);
        num++;
    } while (access(file.c_str(), F_OK) != -1);

    // Get Image Size
    GLint viewport[4];
    media_glGetIntegerv(GL_VIEWPORT, viewport);
    const int x = viewport[0];
    const int y = viewport[1];
    const int width = viewport[2];
    const int height = viewport[3];

    // Get Line Size
    int line_size = width * 4;
    {
        // Handle Alignment
        int alignment;
        media_glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
        // Round
        line_size = align_up(line_size, alignment);
    }
    const int size = height * line_size;

    // Read Pixels
    unsigned char *pixels = new unsigned char[size];
    media_glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Save Image
    if (save_png(file.c_str(), pixels, line_size, width, height)) {
        WARN("Screenshot Failed: %s", file.c_str());
    } else {
        INFO("Screenshot Saved: %s", file.c_str());
        if (gui) {
            std::string chat_msg = "Saved screenshot as ";
            chat_msg += filename;
            gui->addMessage(chat_msg);
        }
    }

    // Free
    delete[] pixels;
}

// Init
void init_screenshot() {
    if (feature_has("Screenshot Support", server_disabled)) {
        // Create Directory
        get_screenshot_dir();
        // Take Screenshot On F2
        misc_run_on_key_press([](Minecraft *mc, const int key) {
            if (key == MC_KEY_F2) {
                screenshot_take(&mc->gui);
                return true;
            } else {
                return false;
            }
        });
    }
}