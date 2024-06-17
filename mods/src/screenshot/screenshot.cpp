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
#include <mods/screenshot/screenshot.h>

// Ensure Screenshots Folder Exists
static void ensure_screenshots_folder(const char *screenshots) {
    // Check Screenshots Folder
    ensure_directory(screenshots);
}

// Take Screenshot
static int save_png(const char *filename, unsigned char *pixels, int line_size, int width, int height) {
    // Setup
    stbi_flip_vertically_on_write(1);

    // Write Image
    return !stbi_write_png(filename, width, height, 4, pixels, line_size);
}
void screenshot_take(const char *home) {
    // Check
    if (reborn_is_headless()) {
        IMPOSSIBLE();
    }

    // Get Directory
    const std::string screenshots = std::string(home) + "/screenshots";

    // Get Timestamp
    time_t raw_time;
    time(&raw_time);
    tm *time_info = localtime(&raw_time);
    char time[512];
    strftime(time, 512, "%Y-%m-%d_%H.%M.%S", time_info);

    // Ensure Screenshots Folder Exists
    ensure_screenshots_folder(screenshots.c_str());

    // Prevent Overwriting Screenshots
    int num = 1;
    std::string file = screenshots + '/' + time + ".png";
    while (access(file.c_str(), F_OK) != -1) {
        file = screenshots + '/' + time + '-' + std::to_string(num) + ".png";
        num++;
    }

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
        int diff = line_size % alignment;
        if (diff > 0) {
            line_size = line_size + (alignment - diff);
        }
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
    }

    // Free
    free(pixels);
}
