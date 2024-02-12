#include <libreborn/libreborn.h>
#include "patch-internal.h"

// Generate A BL Instruction
#define INSTRUCTION_RANGE 32000000
static uint64_t nice_diff_64(uint64_t a, uint64_t b) {
    if (a > b) {
        return a - b;
    } else {
        return b - a;
    }
}
uint32_t generate_bl_instruction(void *from, void *to, int use_b_instruction) {
    // Check Instruction Range
    if (nice_diff_64((uint64_t) to, (uint64_t) from) > INSTRUCTION_RANGE) {
        IMPOSSIBLE();
    }

    // Create New Instruction
    uint32_t instruction;
    unsigned char *instruction_array = (unsigned char *) &instruction;
    instruction_array[3] = use_b_instruction ? B_INSTRUCTION : BL_INSTRUCTION;

    // Determine PC
    unsigned char *pc = ((unsigned char *) from) + 8;
    int32_t offset = (int32_t) to - (int32_t) pc;
    int32_t target = offset >> 2;

    // Set Instruction Offset
    unsigned char *target_array = (unsigned char *) &target;
    instruction_array[0] = target_array[0];
    instruction_array[1] = target_array[1];
    instruction_array[2] = target_array[2];

    // Return
    return instruction;
}

// Extract Target Address From B(L) Instruction
void *extract_from_bl_instruction(unsigned char *from) {
    // Calculate PC
    unsigned char *pc = ((unsigned char *) from) + 8;

    // Extract Offset From Instruction
    int32_t target = *(int32_t *) from;
    target = (target << 8) >> 8;
    int32_t offset = target << 2;
    // Add PC To Offset
    return (void *) (pc + offset);
}
