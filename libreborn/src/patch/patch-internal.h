#pragma once

#ifndef __arm__
#error "Patching Code Is ARM Only"
#endif

#include <cstdint>

// Segments
struct segment_data {
    void *start;
    void *end;
    bool is_executable;
    bool is_writable;
};
__attribute__((visibility("internal"))) segment_data &get_data_for_addr(void *addr);
__attribute__((visibility("internal"))) void add_segment(segment_data data);

// Code Block
__attribute__((visibility("internal"))) void *update_code_block(void *target);
__attribute__((visibility("internal"))) void increment_code_block();

// BL Instruction Magic Number
#define BL_INSTRUCTION 0xeb
#define B_INSTRUCTION 0xea
__attribute__((visibility("internal"))) bool is_branch_instruction(unsigned char opcode);
__attribute__((visibility("internal"))) uint32_t generate_bl_instruction(void *from, void *to, unsigned char opcode = BL_INSTRUCTION);
