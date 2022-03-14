// Screenshot Code Is Useless In Headless Mode
#ifndef MCPI_HEADLESS_MODE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <FreeImage.h>

#include <GLES/gl.h>

#include <libreborn/libreborn.h>
#include <media-layer/core.h>

// Ensure Screenshots Folder Exists
static void ensure_screenshots_folder(char *screenshots) {
    // Check Screenshots Folder
    struct stat obj;
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
void media_take_screenshot(char *home) {
    // Get Directory
    char *screenshots = NULL;
    safe_asprintf(&screenshots, "%s/screenshots", home);

    // Get Timestamp
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char time[TIME_SIZE];
    strftime(time, TIME_SIZE, "%Y-%m-%d_%H.%M.%S", timeinfo);

    // Ensure Screenshots Folder Exists
    ensure_screenshots_folder(screenshots);

    // Prevent Overwriting Screenshots
    int num = 1;
    char *file = NULL;
    safe_asprintf(&file, "%s/%s.png", screenshots, time);
    while (access(file, F_OK) != -1) {
        free(file);
        file = NULL;
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
    int line_size = width * 3;
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
    unsigned char pixels[size];
    glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Handle Little Endian Systems
#if __BYTE_ORDER == __LITTLE_ENDIAN
    // Swap Red And Blue
    for (int j = 0; j < width; j++) {
        for (int k = 0; k < height; k++) {
            int pixel = (k * line_size) + (j * 3);
            // Swap
            int red = pixels[pixel];
            int blue = pixels[pixel + 2];
            pixels[pixel] = blue;
            pixels[pixel + 2] = red;
        }
    }
#endif

    // Save Image
    FIBITMAP *image = FreeImage_ConvertFromRawBits(pixels, width, height, line_size, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, 0);
    if (!FreeImage_Save(FIF_PNG, image, file, 0)) {
        INFO("Screenshot Failed: %s", file);
    } else {
        INFO("Screenshot Saved: %s", file);
    }
    FreeImage_Unload(image);

    // Free
    free(file);
    free(screenshots);
}

// Init
__attribute__((constructor)) static void init() {
    // Init FreeImage
    FreeImage_Initialise(0);
}

#else
void media_take_screenshot() {
    // NOP
}
#endif
