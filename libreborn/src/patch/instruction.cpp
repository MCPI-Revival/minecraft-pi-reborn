#include <libreborn/patch.h>
#include "patch-internal.h"

// Check Instruction
bool is_branch_instruction(unsigned char opcode) {
    // Remove Condition
    opcode &= 0xf;
    // Check
    if (opcode == 0b1010) {
        // B
        return true;
    } else if (opcode == 0b1011) {
        // BL
        return true;
    } else {
        // Not A Branch
        return false;
    }
}

// Extract Target Address From B(L) Instruction
static void *extract_from_bl_instruction(unsigned char *from, const uint32_t instruction) {
    // Extract The Signed 24-Bit Immediate Value
    int32_t imm24 = int32_t(instruction) & 0x00ffffff;
    // Sign-Extend
    if (imm24 & 0x00800000) {
        imm24 |= int32_t(0xff000000);
    }
    // Calculate Offset
    const int32_t offset = imm24 << 2;
    // Compute Target Address
    return from + 8 + offset;
}
void *extract_from_bl_instruction(unsigned char *from) {
    return extract_from_bl_instruction(from, *(uint32_t *) from);
}

// Generate A BL Instruction
uint32_t generate_bl_instruction(void *from, void *to, const unsigned char opcode) {
    const uint32_t from_addr = uint32_t(from);
    const uint32_t to_addr = uint32_t(to);

    // Calculate The Offset
    const int32_t offset = int32_t((to_addr - from_addr - 8) >> 2); // Account For The 2-Bit Shift

    // Create the instruction
    uint32_t instruction = opcode;
    instruction *= 0x1000000;

    // Set The Offset (Last 24 Bits)
    instruction |= (offset & 0x00ffffff);

    // Check
    if (to != extract_from_bl_instruction((unsigned char *) from, instruction)) {
        ERR("Unable To Create Branch Instruction From %p To %p", from, to);
    }

    // Return
    return instruction;
}
