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
static void ensure_screenshots_folder(char *screenshots) {
    // Check Screenshots Folder
    struct stat obj = {};
    if (stat(screenshots, &obj) != 0 || !S_ISDIR(obj.st_mode)) {
        // Create Screenshots Folder
        int ret = mkdir(screenshots, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (ret != 0) {
            // Unable To Create Folder
            ERR("Error Creating Directory: %s: %s", screenshots, strerror(errno));
        }
    }
}

// 4 (Year) + 1 (Hyphen) + 2 (Month) + 1 (Hyphen) + 2 (Day) + 1 (Underscore) + 2 (Hour) + 1 (Period) + 2 (Minute) + 1 (Period) + 2 (Second) + 1 (Null Terminator)
#define TIME_SIZE 20

// Take Screenshot
static int save_png(const char *filename, unsigned char *pixels, int line_size, int width, int height) {
    // Setup
    stbi_flip_vertically_on_write(1);

    // Write Image
    return !stbi_write_png(filename, width, height, 4, pixels, line_size);
}
void screenshot_take(char *home) {
    // Get Directory
    char *screenshots = nullptr;
    safe_asprintf(&screenshots, "%s/screenshots", home);

    // Get Timestamp
    time_t rawtime;
    tm *timeinfo = {};
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char time[TIME_SIZE];
    strftime(time, TIME_SIZE, "%Y-%m-%d_%H.%M.%S", timeinfo);

    // Ensure Screenshots Folder Exists
    ensure_screenshots_folder(screenshots);

    // Prevent Overwriting Screenshots
    int num = 1;
    char *file = nullptr;
    safe_asprintf(&file, "%s/%s.png", screenshots, time);
    while (access(file, F_OK) != -1) {
        free(file);
        file = nullptr;
        safe_asprintf(&file, "%s/%s-%i.png", screenshots, time, num);
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
    if (save_png(file, pixels, line_size, width, height)) {
        WARN("Screenshot Failed: %s", file);
    } else {
        INFO("Screenshot Saved: %s", file);
    }

    // Free
    free(file);
    free(screenshots);
    free(pixels);
}

