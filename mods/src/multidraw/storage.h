#pragma once

#include <vector>
#include <unordered_map>

#include "buffer.h"

// Default Size Of Various Buffers
#define DEFAULT_BUFFER_SIZE 16777216 // 16 MiB

// Block Of Data
struct Block {
    int chunk_id = -1;
    intptr_t offset = 0;
    ssize_t size = 0;
    // Equals
    bool operator==(const Block &other) const {
        return offset == other.offset;
    }
};

// Storage
struct Storage {
    Storage();
    ~Storage();

    // Buffer
    Buffer *buffer;

    // Chunks
    std::unordered_map<int, intptr_t> chunk_to_offset = {};
    void upload(int chunk, ssize_t size, const void *data);

    // Management
    ssize_t total_size;
    ssize_t used_size;
    std::vector<Block> free_blocks = {};
    std::vector<Block> used_blocks = {};
    void free_block(Block block);
    void recreate(ssize_t extra_size = 0);
    void check_fragmentation();
    [[nodiscard]] ssize_t get_end() const;
};