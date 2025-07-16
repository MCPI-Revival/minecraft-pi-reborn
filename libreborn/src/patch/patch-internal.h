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
MCPI_INTERNAL segment_data &get_data_for_addr(void *addr);
MCPI_INTERNAL void add_segment(segment_data data);

// Code Block
MCPI_INTERNAL void *update_code_block(void *target);
MCPI_INTERNAL void increment_code_block();

// BL Instruction Magic Number
#define BL_INSTRUCTION 0xeb
#define B_INSTRUCTION 0xea
MCPI_INTERNAL bool is_branch_instruction(unsigned char opcode);
MCPI_INTERNAL unsigned char get_opcode(void *addr);
MCPI_INTERNAL uint32_t generate_bl_instruction(void *from, void *to, unsigned char opcode = BL_INSTRUCTION);

// Cache
MCPI_INTERNAL void add_callsite(void *addr);
MCPI_INTERNAL void remove_callsite(void *addr);
MCPI_INTERNAL std::unordered_set<void *> get_normal_callsites(void *addr);
MCPI_INTERNAL std::unordered_set<void *> get_virtual_callsites(void *addr);
MCPI_INTERNAL void init_cache();