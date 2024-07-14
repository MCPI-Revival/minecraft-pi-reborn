#include <sys/mman.h>

#include <libreborn/libreborn.h>
#include "patch-internal.h"

// Limit To 512 overwrite_calls() Uses
#define CODE_BLOCK_SIZE 4096
static unsigned char *code_block = nullptr;
#define CODE_SIZE 8
static int code_block_remaining = CODE_BLOCK_SIZE;

// Create Long Overwrite At Current Position
static void long_overwrite(void *start, void *target) {
    unsigned char patch_data[4] = {0x04, 0xf0, 0x1f, 0xe5}; // "ldr pc, [pc, #-0x4]"
    patch(start, patch_data);
    patch_address((void *) (((unsigned char *) start) + 4), target);
}
void *update_code_block(void *target) {
    // BL Instructions can only access a limited portion of memory.
    // So this allocates memory closer to the original instruction,
    // that when run, will jump into the actual target.
    if (code_block == nullptr) {
        code_block = (unsigned char *) mmap((void *) 0x200000, CODE_BLOCK_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (code_block == MAP_FAILED) {
            ERR("Unable To Allocate Code Block: %s", strerror(errno));
        }
        DEBUG("Code Block Allocated At: 0x%08x", (uint32_t) code_block);
        // Store Segment
        segment_data data = {};
        data.start = code_block;
        data.end = (void *) (((uintptr_t) code_block) + CODE_BLOCK_SIZE);
        data.is_executable = true;
        data.is_writable = true;
        add_segment(data);
    }
    if (code_block_remaining < CODE_SIZE) {
        ERR("Maximum Amount Of overwrite_calls() Uses Reached");
    }
    long_overwrite(code_block, target);
    // Return
    return code_block;
}
void increment_code_block() {
    code_block = code_block + CODE_SIZE;
    code_block_remaining = code_block_remaining - CODE_SIZE;
}
