#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <ctime>
#include <cstring>
#include <cerrno>
#include <sys/stat.h>

#include "stb_image.h"
#include "stb_image_write.h"

#include <libreborn/libreborn.h>
#include <GLES/gl.h>
#include <symbols/minecraft.h>

#include <mods/screenshot/screenshot.h>
#include <mods/home/home.h>
#include <mods/misc/misc.h>
#include <mods/input/input.h>
#include <mods/init/init.h>

// Ensure Screenshots Folder Exists
static std::string get_screenshot_dir() {
    std::string dir = std::string(home_get()) + "/screenshots";
    ensure_directory(dir.c_str());
    return dir;
}
static std::string get_screenshot(const std::string &filename) {
    return get_screenshot_dir() + '/' + filename;
}

// Take Screenshot
static int save_png(const char *filename, unsigned char *pixels, int line_size, int width, int height) {
    // Setup
    stbi_flip_vertically_on_write(1);

    // Write Image
    return !stbi_write_png(filename, width, height, 4, pixels, line_size);
}
void screenshot_take(Gui *gui) {
    // Check
    if (reborn_is_headless()) {
        IMPOSSIBLE();
    }

    // Get Timestamp
    time_t raw_time;
    time(&raw_time);
    tm *time_info = localtime(&raw_time);
    char time[512];
    strftime(time, 512, "%Y-%m-%d_%H.%M.%S", time_info);

    // Prevent Overwriting Screenshots
    int num = 0;
    std::string filename;
    do {
        filename = std::string(time);
        if (num > 0) {
            filename += '-' + std::to_string(num);
        }
        filename += ".png";
        num++;
    } while (access(get_screenshot(filename).c_str(), F_OK) != -1);
    const std::string file = get_screenshot(filename);

    // Get Image Size
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int x = viewport[0];
    int y = viewport[1];
    int width = viewport[2];
    int height = viewport[3];

    // Get Line Size
    int line_size = width * 4;
    {
        // Handle Alignment
        int alignment;
        glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
        // Round
        line_size = ALIGN_UP(line_size, alignment);
    }
    int size = height * line_size;

    // Read Pixels
    unsigned char *pixels = (unsigned char *) malloc(size);
    ALLOC_CHECK(pixels);
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Save Image
    if (save_png(file.c_str(), pixels, line_size, width, height)) {
        WARN("Screenshot Failed: %s", file.c_str());
    } else {
        INFO("Screenshot Saved: %s", file.c_str());
        if (gui) {
            std::string chat_msg = "Saved screenshot as ";
            chat_msg += filename;
            gui->addMessage(&chat_msg);
        }
    }

    // Free
    free(pixels);
}

// Init
void init_screenshot() {
    // Create Directory
    get_screenshot_dir();
    // Take Screenshot On F2
    misc_run_on_key_press([](Minecraft *mc, int key) {
        if (key == MC_KEY_F2) {
            screenshot_take(&mc->gui);
            return true;
        } else {
            return false;
        }
    });
}