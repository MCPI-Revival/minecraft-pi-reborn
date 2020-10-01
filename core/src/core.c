#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>

#include <libcore/libcore.h>

#define PATCH_PRINTF(print, start, str) if (print) fprintf(stderr, "Patching (0x%04x) - "str": 0x%02x 0x%02x 0x%02x 0x%02x\n", (uint32_t) start, data[0], data[1], data[2], data[3]);

#define PREPARE_PATCH(print, count) \
    size_t page_size = sysconf(_SC_PAGESIZE); \
    uintptr_t end = ((uintptr_t) start) + (4 * count); \
    uintptr_t page_start = ((uintptr_t) start) & -page_size; \
    mprotect((void *) page_start, end - page_start, PROT_READ | PROT_WRITE); \
    \
    unsigned char *data = (unsigned char *) start; \
    int thumb = ((size_t) start) & 1; \
    if (thumb) { \
        data--; \
    } \
    PATCH_PRINTF(print, start, "original");

#define END_PATCH(print) \
    PATCH_PRINTF(print, start, "result"); \
    \
    mprotect((void *) page_start, end - page_start, PROT_READ | PROT_EXEC); \
    __clear_cache(start, (void *) end);

#define ORIGINAL_SIZE 4 + sizeof (int)

void *overwrite(void *start, void *target) {
    PREPARE_PATCH(1, 2);

    void *original = malloc(ORIGINAL_SIZE);
    memcpy(original, start, ORIGINAL_SIZE);

    if (thumb) {
        unsigned char patch[4] = {0xdf, 0xf8, 0x00, 0xf0};
        memcpy(data, patch, 4);
    } else {
        unsigned char patch[4] = {0x04, 0xf0, 0x1f, 0xe5};
        memcpy(data, patch, 4);
    }
    memcpy(&data[4], &target, sizeof (int));

    END_PATCH(1);

    return original;
}

void revert_overwrite(void *start, void *original) {
    PREPARE_PATCH(0, 2);

    void *temp = malloc(ORIGINAL_SIZE);
    memcpy(temp, data, ORIGINAL_SIZE);
    memcpy(data, original, ORIGINAL_SIZE);
    memcpy(original, temp, ORIGINAL_SIZE);
    free(temp);

    END_PATCH(0);
}

void patch(void *start, unsigned char patch[]) {
    PREPARE_PATCH(1, 1);
    memcpy(data, patch, 4);
    END_PATCH(1);
}
