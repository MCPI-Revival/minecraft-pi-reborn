#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>

#include <libcore/libcore.h>

#define PREPARE_PATCH(start) \
    size_t page_size = sysconf(_SC_PAGESIZE); \
    uintptr_t end = ((uintptr_t) start) + 1; \
    uintptr_t page_start = ((uintptr_t) start) & -page_size; \
    mprotect((void *) page_start, end - page_start, PROT_READ | PROT_WRITE); \
    \
    unsigned char *data = (unsigned char *) start; \
    int thumb = ((size_t) start) & 1; \
    if (thumb) { \
        data--; \
    } \
    fprintf(stderr, "Patching - original: %i %i %i %i %i\n", data[0], data[1], data[2], data[3], data[4]);

#define END_PATCH() \
    fprintf(stderr, "Patching - result: %i %i %i %i %i\n", data[0], data[1], data[2], data[3], data[4]); \
    \
    mprotect((void *) page_start, end - page_start, PROT_READ | PROT_EXEC);

void overwrite(void *start, void *target) {
    PREPARE_PATCH(start);
    if (thumb) {
        unsigned char patch[4] = {0xdf, 0xf8, 0x00, 0xf0};
        memcpy(data, patch, 4);
    } else {
        unsigned char patch[4] = {0x04, 0xf0, 0x1f, 0xe5};
        memcpy(data, patch, 4);
    }
    memcpy(&data[4], &target, sizeof (int));
    END_PATCH();
}

void patch(void *start, unsigned char patch[]) {
    PREPARE_PATCH(start);
    memcpy(data, patch, 4);
    END_PATCH();
}
