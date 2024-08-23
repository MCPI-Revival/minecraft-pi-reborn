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
static void _overwrite_call_internal(void *start, void *target, const bool use_b_instruction) {
    // Add New Target To Code Block
    void *code_block = update_code_block(target);

    // Patch
    uint32_t new_instruction = generate_bl_instruction(start, code_block, use_b_instruction ? B_INSTRUCTION : BL_INSTRUCTION);
    patch(start, (unsigned char *) &new_instruction);

    // Increment Code Block Position
    increment_code_block();
}
void overwrite_call(void *start, void *target) {
    const bool use_b_instruction = ((unsigned char *) start)[3] == B_INSTRUCTION;
    _overwrite_call_internal(start, target, use_b_instruction);
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
            patch_address(addr, replacement); \
            found++; \
        } \
    }
static int _patch_vtables(void *target, void *replacement) {
    int found = 0;
    scan_vtables(RODATA);
    scan_vtables(DATA_REL_RO);
    return found;
}
#undef scan_vtables

// Patch Calls Within Range
static int _overwrite_calls_within_internal(void *from, void *to, void *target, void *replacement) {
    int found = 0;
    for (uintptr_t i = (uintptr_t) from; i < (uintptr_t) to; i = i + 4) {
        unsigned char *addr = (unsigned char *) i;
        const unsigned char opcode = addr[3];
        // Check If Instruction is B Or BL
        if (is_branch_instruction(opcode)) {
            // Extract Instruction Target
            const void *instruction_target = extract_from_bl_instruction(addr);
            // Check If Instruction Calls Target
            if (instruction_target == target) {
                // Patch Instruction
                uint32_t new_instruction = generate_bl_instruction(addr, replacement, opcode);
                patch(addr, (unsigned char *) &new_instruction);
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
#define NO_CALLSITE_ERROR() ERR("Unable To Find Callsites")
void *overwrite_calls_manual(void *start, void *target, const bool allow_no_callsites) {
    // Add New Target To Code Block
    void *code_block = update_code_block(target);

    // Patch Code
    int found = _overwrite_calls_within_internal((void *) TEXT_START, (void *) TEXT_END, start, code_block);
    // Patch VTables
    found += _patch_vtables(start, code_block);

    // Increment Code Block Position
    increment_code_block();

    // Check
    if (found < 1 && !allow_no_callsites) {
        NO_CALLSITE_ERROR();
    }

    // Return
    return code_block;
}
void overwrite_calls_within_manual(void *from /* inclusive */, void *to /* exclusive */, void *target, void *replacement) {
    // Add New Target To Code Block
    void *code_block = update_code_block(replacement);

    // Patch
    const int found = _overwrite_calls_within_internal(from, to, target, code_block);
    // Check
    if (found < 1) {
        NO_CALLSITE_ERROR();
    }

    // Increment Code Block Position
    increment_code_block();
}

// Patch Instruction
static void safe_mprotect(void *addr, const size_t len, const int prot) {
    const long page_size = sysconf(_SC_PAGESIZE);
    const long diff = uintptr_t(addr) % page_size;
    void *aligned_addr = (void *) (((uintptr_t) addr) - diff);
    const size_t aligned_len = len + diff;
    const int ret = mprotect(aligned_addr, aligned_len, prot);
    if (ret == -1) {
        ERR("Unable To Set Permissions: %p: %s", addr, strerror(errno));
    }
}
void patch(void *start, unsigned char patch[4]) {
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
    const uint32_t size = 4;
    safe_mprotect(start, size, prot | PROT_WRITE);

    // Patch
    unsigned char *data = (unsigned char *) start;
    memcpy(data, patch, 4);

    // Reset Code Memory Permissions
    safe_mprotect(start, size, prot);

    // Clear ARM Instruction Cache
    __clear_cache(start, (void *) (((uintptr_t) start) + size));
}

// Patch Address
void patch_address(void *start, void *target) {
    uint32_t addr = (uint32_t) target;
    unsigned char *patch_data = (unsigned char *) &addr;
    patch(start, patch_data);
}

// Thunks
void *reborn_thunk_enabler(void *target, void *thunk) {
    return overwrite_calls_manual(target, thunk, true);
}