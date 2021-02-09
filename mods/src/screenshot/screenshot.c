#define _GNU_SOURCE

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

#include "screenshot.h"

// 4 (Year) + 1 (Hyphen) + 2 (Month) + 1 (Hyphen) + 2 (Day) + 1 (Underscore) + 2 (Hour) + 1 (Period) + 2 (Minute) + 1 (Period) + 2 (Second) + 1 (Null Terminator)
#define TIME_SIZE 20

// Take Screenshot
void take_screenshot() {
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char time[TIME_SIZE];
    strftime(time, TIME_SIZE, "%Y-%m-%d_%H.%M.%S", timeinfo);

    char *screenshots = NULL;
    asprintf(&screenshots, "%s/.minecraft/screenshots", getenv("HOME"));

    int num = 1;
    char *file = NULL;
    asprintf(&file, "%s/%s.png", screenshots, time);
    while (access(file, F_OK) != -1) {
        asprintf(&file, "%s/%s-%i.png", screenshots, time, num);
        num++;
    }

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int x = viewport[0];
    int y = viewport[1];
    int width = viewport[2];
    int height = viewport[3];

    int line_size = width * 3;
    int size = height * line_size;

    unsigned char pixels[size];
    glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    // Swap Red And Blue
    for (int i = 0; i < (size / 3); i++) {
        int pixel = i * 3;
        int red = pixels[pixel];
        int blue = pixels[pixel + 2];
        pixels[pixel] = blue;
        pixels[pixel + 2] = red;
    }
#endif

    FIBITMAP *image = FreeImage_ConvertFromRawBits(pixels, width, height, line_size, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, 0);
    if (!FreeImage_Save(FIF_PNG, image, file, 0)) {
        INFO("Screenshot Failed: %s", file);
    } else {
        INFO("Screenshot Saved: %s", file);
    }
    FreeImage_Unload(image);

    free(file);
    free(screenshots);
}

// Init
__attribute__((constructor)) static void init() {
    // Init FreeImage
    FreeImage_Initialise(0);

    // Screenshots Folder
    char *screenshots_folder = NULL;
    asprintf(&screenshots_folder, "%s/.minecraft/screenshots", getenv("HOME"));
    {
        // Check Screenshots Folder
        struct stat obj;
        if (stat(screenshots_folder, &obj) != 0 || !S_ISDIR(obj.st_mode)) {
            // Create Screenshots Folder
            int ret = mkdir(screenshots_folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (ret != 0) {
                // Unable To Create Folder
                ERR("Error Creating Directory: %s: %s", screenshots_folder, strerror(errno));
            }
        }
    }
    free(screenshots_folder);
}