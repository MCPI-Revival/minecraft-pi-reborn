#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdint>
#include <cerrno>
#include <unordered_set>

#include <libreborn/patch.h>
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
void overwrite_call_manual(void *addr, void *new_target, const bool force_b_instruction) {
    const bool use_b_instruction = force_b_instruction || ((unsigned char *) addr)[3] == B_INSTRUCTION;
    _overwrite_call_internal(addr, new_target, use_b_instruction);
}

// Patch VTables
static int _patch_vtables(void *target, void *replacement) {
    int found = 0;
    const std::unordered_set<void *> callsites = get_virtual_callsites(target);
    for (void *addr : callsites) {
        patch_address(addr, replacement);
        found++;
    }
    return found;
}

// Patch Calls Within Range
static int _overwrite_calls_within_internal(const void *from, const void *to, void *target, void *replacement) {
    int found = 0;
    const std::unordered_set<void *> callsites = get_normal_callsites(target);
    for (void *addr : callsites) {
        if (to == nullptr || (addr >= from && addr < to)) {
            const unsigned char opcode = get_opcode(addr);
            uint32_t new_instruction = generate_bl_instruction(addr, replacement, opcode);
            patch(addr, (unsigned char *) &new_instruction);
            found++;
        }
    }
    return found;
}

// Overwrite All B(L) Instructions That Target The Specified Address
#define NO_CALLSITE_ERROR() ERR("Unable To Find Callsites")
void *overwrite_calls_manual(void *target, void *replacement, const bool allow_no_callsites) {
    // Add New Target To Code Block
    void *code_block = update_code_block(replacement);

    // Patch Code
    int found = _overwrite_calls_within_internal(nullptr, nullptr, target, code_block);
    // Patch VTables
    found += _patch_vtables(target, code_block);

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
    const uintptr_t diff = uintptr_t(addr) % page_size;
    void *aligned_addr = (void *) (uintptr_t(addr) - diff);
    const size_t aligned_len = len + diff;
    const int ret = mprotect(aligned_addr, aligned_len, prot);
    if (ret == -1) {
        ERR("Unable To Set Memory Permissions: %p: %s", addr, strerror(errno));
    }
}
static bool record_patch = true;
bool ignore_patch_conflict = false;
void patch(void *addr, unsigned char patch[4]) {
    if (uint32_t(addr) % 4 != 0) {
        ERR("Invalid Address: %p", addr);
    }

    // Detect Multiple Patches At Address
    static std::unordered_set<void *> patched_addresses;
    if (patched_addresses.contains(addr) && !ignore_patch_conflict) {
        ERR("Patch Conflict Detected: %p", addr);
    }
    if (record_patch) {
        patched_addresses.insert(addr);
    }

    // Get Current Permissions
    const segment_data &segment_data = get_data_for_addr(addr);
    int prot = PROT_READ;
    if (segment_data.is_executable) {
        prot |= PROT_EXEC;
    }
    if (segment_data.is_writable) {
        prot |= PROT_WRITE;
    }

    // Allow Writing To Code Memory
    constexpr uint32_t size = 4;
    safe_mprotect(addr, size, prot | PROT_WRITE);

    // Patch
    remove_callsite(addr);
    unsigned char *data = (unsigned char *) addr;
    memcpy(data, patch, 4);
    add_callsite(addr);

    // Reset Code Memory Permissions
    safe_mprotect(addr, size, prot);

    // Clear ARM Instruction Cache
    __clear_cache(addr, (void *) (((uintptr_t) addr) + size));
}

// Patch Address
void patch_address(void *addr, void *target) {
    uint32_t target_addr = (uint32_t) target;
    unsigned char *patch_data = (unsigned char *) &target_addr;
    patch(addr, patch_data);
}

// Thunks
void *reborn_thunk_enabler(void *target, void *thunk) {
    // Treat These Patches As If They Were Part Of The Original Binary
    record_patch = false;
    ignore_patch_conflict = true;
    void *ret = overwrite_calls_manual(target, thunk, true);
    record_patch = true;
    ignore_patch_conflict = false;
    return ret;
}