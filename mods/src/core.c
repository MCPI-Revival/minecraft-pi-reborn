#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>

#include <libcore/libcore.h>

#define PATCH_PRINTF(file, line, start, str) if (file != NULL) fprintf(stderr, "[%s:%i] Patching (0x%04x) - "str": 0x%02x 0x%02x 0x%02x 0x%02x\n", file, line, (uint32_t) start, data[0], data[1], data[2], data[3]);

#define ORIGINAL_SIZE 8

void *_overwrite(const char *file, int line, void *start, void *target) {
    void *original = malloc(ORIGINAL_SIZE);
    memcpy(original, start, ORIGINAL_SIZE);

    int thumb = ((uint32_t) start) & 1;
    unsigned char *patch_data;
    if (thumb) {
        unsigned char patch_data_temp[4] = {0xdf, 0xf8, 0x00, 0xf0};
        patch_data = patch_data_temp;
    } else {
        unsigned char patch_data_temp[4] = {0x04, 0xf0, 0x1f, 0xe5};
        patch_data = patch_data_temp;
    }

    _patch(file, line, start, patch_data);
    _patch(file, line, start + 4, (unsigned char *) &target);

    return original;
}

void revert_overwrite(void *start, void *original) {
    unsigned char *data = (unsigned char *) start;
    int thumb = ((uint32_t) start) & 1;
    if (thumb) {
        data--;
    }

    // Store Current Value In Temp
    void *temp = malloc(ORIGINAL_SIZE);
    memcpy(temp, data, ORIGINAL_SIZE);

    // Insert Original Value
    _patch(NULL, -1, start, original);
    _patch(NULL, -1, start + 4, original + 4);

    // Complete Memory Swap
    memcpy(original, temp, ORIGINAL_SIZE);
    free(temp);
}

void _patch(const char *file, int line, void *start, unsigned char patch[]) {
    size_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t end = ((uintptr_t) start) + 4;
    uintptr_t page_start = ((uintptr_t) start) & -page_size;
    mprotect((void *) page_start, end - page_start, PROT_READ | PROT_WRITE);

    unsigned char *data = (unsigned char *) start;
    int thumb = ((uint32_t) start) & 1;
    if (thumb) {
        data--;
    }

    PATCH_PRINTF(file, line, start, "original");

    memcpy(data, patch, 4);

    PATCH_PRINTF(file, line, start, "result");

    mprotect((void *) page_start, end - page_start, PROT_READ | PROT_EXEC);

    __clear_cache(start, end);
}

void _patch_address(const char *file, int line, void *start, void *target) {
    uint32_t addr = (uint32_t) target;
    unsigned char patch_data[4] = {addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff, (addr >> 24) & 0xff};
    _patch(file, line, start, patch_data);
}
