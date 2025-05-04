#pragma once

#include <vector>

#include "buffer.h"

// Block Of Data
struct Block {
    intptr_t offset;
    ssize_t size;
    // Prevent Copying
    Block() {
        offset = 0;
        size = 0;
    }
    Block(const Block &) = delete;
    Block &operator=(const Block &) = delete;
};

// Storage
struct Storage {
    explicit Storage(int chunks);
    ~Storage();

    // Buffer
    Buffer *buffer;

    // Chunks
    std::vector<Block *> chunk_to_block = {};
    void upload(int chunk, ssize_t size, const void *data);

    // Management
    ssize_t total_size;
    ssize_t used_size;
    std::vector<Block *> free_blocks = {};
    std::vector<Block *> used_blocks = {};
    void free_block(Block *block);
    void recreate(ssize_t extra_size = 0);
    void check_fragmentation();
    [[nodiscard]] ssize_t get_end() const;
};