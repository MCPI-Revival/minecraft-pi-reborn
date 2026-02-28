#pragma once

#ifndef __arm__
#error "Patching Code Is ARM Only"
#endif

#include <cstdint>
#include <unordered_set>

// Segments
struct segment_data {
    void *start;
    void *end;
    bool is_executable;
    bool is_writable;
};
segment_data &get_data_for_addr(void *addr);
void add_segment(segment_data data);

// Code Block
void *update_code_block(void *target);
void increment_code_block();

// BL Instruction Magic Number
#define BL_INSTRUCTION 0xeb
#define B_INSTRUCTION 0xea
bool is_branch_instruction(unsigned char opcode);
unsigned char get_opcode(void *addr);
uint32_t generate_bl_instruction(void *from, void *to, unsigned char opcode = BL_INSTRUCTION);

// Cache
void add_callsite(void *addr);
void remove_callsite(void *addr);
std::unordered_set<void *> get_normal_callsites(void *addr);
std::unordered_set<void *> get_virtual_callsites(void *addr);
void init_cache();