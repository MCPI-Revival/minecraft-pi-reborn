// Config Needs To Load First
#include <libreborn/libreborn.h>

// Screenshot Code Is Useless In Headless Mode
#ifndef MCPI_HEADLESS_MODE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <png.h>

#include <GLES/gl.h>
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
static int save_png(const char *filename, unsigned char *pixels, int line_size, int width, int height) {
    // Return value
    int ret = 0;

    // Variables
    png_structp png = NULL;
    png_infop info = NULL;
    FILE *file = NULL;
    png_colorp palette = NULL;
    png_bytep rows[height];
    for (int i = 0; i < height; ++i) {
        rows[height - i - 1] = (png_bytep) (&pixels[i * line_size]);
    }

    // Init
    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        ret = 1;
        goto ret;
    }
    info = png_create_info_struct(png);
    if (!info) {
        ret = 1;
        goto ret;
    }

    // Open File
    file = fopen(filename, "wb");
    if (!file) {
        ret = 1;
        goto ret;
    }

    // Prepare To Write
    png_init_io(png, file);
    png_set_IHDR(png, info, width, height, 8 /* Depth */, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    palette = (png_colorp) png_malloc(png, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
    if (!palette) {
        ret = 1;
        goto ret;
    }
    png_set_PLTE(png, info, palette, PNG_MAX_PALETTE_LENGTH);
    png_write_info(png, info);
    png_set_packing(png);

    // Write
    png_write_image(png, rows);
    png_write_end(png, info);

 ret:
    // Free
    if (palette != NULL) {
        png_free(png, palette);
    }
    if (file != NULL) {
        fclose(file);
    }
    if (png != NULL) {
        png_destroy_write_struct(&png, &info);
    }

    // Return
    return ret;
}
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
    unsigned char pixels[size];
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Save Image
    if (save_png(file, pixels, line_size, width, height)) {
        INFO("Screenshot Failed: %s", file);
    } else {
        INFO("Screenshot Saved: %s", file);
    }

    // Free
    free(file);
    free(screenshots);
}

#else
void media_take_screenshot() {
    // NOP
}
#endif
