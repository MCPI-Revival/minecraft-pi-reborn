#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdint>
#include <cerrno>

#include <libreborn/libreborn.h>
#include "patch-internal.h"

// Overwrite Specific B(L) Instruction
static void _overwrite_call_internal(const char *file, int line, void *start, void *target, int use_b_instruction) {
    // Add New Target To Code Block
    void *code_block = update_code_block(target);

    // Patch
    uint32_t new_instruction = generate_bl_instruction(start, code_block, use_b_instruction);
    _patch(file, line, start, (unsigned char *) &new_instruction);

    // Increment Code Block Position
    increment_code_block();
}
void _overwrite_call(const char *file, int line, void *start, void *target) {
    int use_b_instruction = ((unsigned char *) start)[3] == B_INSTRUCTION;
    _overwrite_call_internal(file, line, start, target, use_b_instruction);
}

// .rodata Information
#define RODATA_START 0x1020c8
#define RODATA_END 0x11665c
// .data.rel.ro Information
#define DATA_REL_RO_START 0x1352b8
#define DATA_REL_RO_END 0x135638
// Search And Patch VTables Containing Function
#define scan_vtables(section) \
    for (uintptr_t i = section##_START; i < section##_END; i = i + 4) { \
        uint32_t *addr = (uint32_t *) i; \
        if (*addr == (uintptr_t) target) { \
            /* Found VTable Entry */ \
            _patch_address(file, line, addr, replacement); \
            found++; \
        } \
    }
static int _patch_vtables(const char *file, int line, void *target, void *replacement) {
    int found = 0;
    scan_vtables(RODATA);
    scan_vtables(DATA_REL_RO);
    return found;
}
#undef scan_vtables

// Patch Calls Within Range
static int _overwrite_calls_within_internal(const char *file, int line, void *from, void *to, void *target, void *replacement) {
    int found = 0;
    for (uintptr_t i = (uintptr_t) from; i < (uintptr_t) to; i = i + 4) {
        unsigned char *addr = (unsigned char *) i;
        int use_b_instruction = addr[3] == B_INSTRUCTION;
        // Check If Instruction is B Or BL
        if (addr[3] == BL_INSTRUCTION || use_b_instruction) {
            uint32_t check_instruction = generate_bl_instruction(addr, target, use_b_instruction);
            unsigned char *check_instruction_array = (unsigned char *) &check_instruction;
            // Check If Instruction Calls Target
            if (addr[0] == check_instruction_array[0] && addr[1] == check_instruction_array[1] && addr[2] == check_instruction_array[2]) {
                // Patch Instruction
                uint32_t new_instruction = generate_bl_instruction(addr, replacement, use_b_instruction);
                _patch(file, line, addr, (unsigned char *) &new_instruction);
                found++;
            }
        }
    }
    return found;
}

// .text Information
#define TEXT_START 0xde60
#define TEXT_END 0x1020c0
// Overwrite All B(L) Intrusctions That Target The Specified Address
#define NO_CALLSITE_ERROR "(%s:%i) Unable To Find Callsites For %p"
void _overwrite_calls(const char *file, int line, void *start, void *target) {
    // Add New Target To Code Block
    void *code_block = update_code_block(target);

    // Patch Code
    int found = _overwrite_calls_within_internal(file, line, (void *) TEXT_START, (void *) TEXT_END, start, code_block);
    // Patch VTables
    found += _patch_vtables(file, line, start, code_block);

    // Increment Code Block Position
    increment_code_block();

    // Check
    if (found < 1) {
        ERR(NO_CALLSITE_ERROR, file, line, start);
    }
}
void _overwrite_calls_within(const char *file, int line, void *from /* inclusive */, void *to /* exclusive */, void *target, void *replacement) {
    // Add New Target To Code Block
    void *code_block = update_code_block(replacement);

    // Patch
    int found = _overwrite_calls_within_internal(file, line, from, to, target, code_block);
    // Check
    if (found < 1) {
        ERR(NO_CALLSITE_ERROR, file, line, target);
    }

    // Increment Code Block Position
    increment_code_block();
}

// Overwrite Function
void _overwrite(const char *file, int line, void *start, void *target) {
    // Replace the function's start with a call
    // to the replacement function.
    _overwrite_call_internal(file, line, start, target, 1);
}

// Print Patch Debug Data
#define PATCH_PRINTF(file, line, start, str) if (file != NULL) DEBUG("(%s:%i): Patching (%p) - " str ": %02x %02x %02x %02x", file, line, start, data[0], data[1], data[2], data[3]);

// Patch Instruction
static void safe_mprotect(void *addr, size_t len, int prot) {
    long page_size = sysconf(_SC_PAGESIZE);
    long diff = ((uintptr_t) addr) % page_size;
    void *aligned_addr = (void *) (((uintptr_t) addr) - diff);
    size_t aligned_len = len + diff;
    int ret = mprotect(aligned_addr, aligned_len, prot);
    if (ret == -1) {
        ERR("Unable To Set Permissions: %p: %s", addr, strerror(errno));
    }
}
void _patch(const char *file, int line, void *start, unsigned char patch[4]) {
    if (((uint32_t) start) % 4 != 0) {
        ERR("Invalid Address: %p", start);
    }

    // Get Current Permissions
    segment_data &segment_data = get_data_for_addr(start);
    int prot = PROT_READ;
    if (segment_data.is_executable) {
        prot |= PROT_EXEC;
    }
    if (segment_data.is_writable) {
        prot |= PROT_WRITE;
    }

    // Allow Writing To Code Memory
    uint32_t size = 4;
    safe_mprotect(start, size, prot | PROT_WRITE);

    // Patch
    unsigned char *data = (unsigned char *) start;
    PATCH_PRINTF(file, line, start, "original");
    memcpy(data, patch, 4);
    PATCH_PRINTF(file, line, start, "result");

    // Reset Code Memory Permissions
    safe_mprotect(start, size, prot);

    // Clear ARM Instruction Cache
    __clear_cache(start, (void *) (((uintptr_t) start) + size));
}

// Patch Address
void _patch_address(const char *file, int line, void *start, void *target) {
    uint32_t addr = (uint32_t) target;
    unsigned char *patch_data = (unsigned char *) &addr;
    _patch(file, line, start, patch_data);
}
