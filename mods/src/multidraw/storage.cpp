#include <algorithm>

#include "storage.h"

// Setup
Storage::Storage() {
    // Allocate
    total_size = DEFAULT_BUFFER_SIZE;
    used_size = 0;
    buffer = new Buffer(total_size);
    // First Free Block
    Block block;
    block.offset = 0;
    block.size = total_size;
    free_blocks.push_back(block);
}
Storage::~Storage() {
    delete buffer;
}

// Free Block
void Storage::free_block(Block block) {
    // Check
    if (block.size == 0) {
        return;
    }

    // Find Block
    std::vector<Block>::iterator it = std::ranges::find(used_blocks, block);
    if (it == used_blocks.end()) {
        return;
    }
    used_blocks.erase(it);

    // Update Size
    used_size -= block.size;

    // Merge With Next/Previous Blocks
    for (it = free_blocks.begin(); it < free_blocks.end(); ++it) {
        const Block &x = *it;
        if (x.offset == (block.offset + block.size)) {
            // Found Free Block After Target
            block.size += x.size;
            free_blocks.erase(it);
            break;
        }
    }
    for (it = free_blocks.begin(); it < free_blocks.end(); ++it) {
        const Block &x = *it;
        if ((x.offset + x.size) == block.offset) {
            // Found Free Block Before Target
            block.size += x.size;
            block.offset -= x.size;
            free_blocks.erase(it);
            break;
        }
    }

    // Add To Free Block List
    free_blocks.push_back(block);

    // Fragmentation
    check_fragmentation();
}

// Check Fragmentation
ssize_t Storage::get_end() const {
    ssize_t end = 0;
    for (const Block &block : used_blocks) {
        const ssize_t new_end = block.offset + block.size;
        if (new_end > end) {
            end = new_end;
        }
    }
    return end;
}
void Storage::check_fragmentation() {
    const ssize_t end = get_end();
    if (end == 0) {
        return;
    }
    const float fragmentation = 1.0f - (float(used_size) / float(end));
    if (fragmentation >= 0.5f) {
        // 50% Fragmentation
        recreate();
    }
}

// Upload Chunk
void Storage::upload(const int chunk, const ssize_t size, const void *data) {
    // Free Old Block
    if (chunk_to_offset.contains(chunk)) {
        const intptr_t old_offset = chunk_to_offset[chunk];
        for (const Block &block : used_blocks) {
            if (block.offset == old_offset) {
                free_block(block);
                break;
            }
        }
    }

    // Check Size
    if (size == 0) {
        return;
    }

    // Find Free Block
    std::vector<Block>::iterator it;
    while (true) {
        for (it = free_blocks.begin(); it < free_blocks.end(); ++it) {
            if (it->size >= size) {
                break;
            }
        }
        if (it == free_blocks.end()) {
            // Get Extra Space
            recreate(size);
        } else {
            // Done!
            break;
        }
    }
    Block &old_free_block = *it;

    // Create New Used Block
    Block new_used_block;
    new_used_block.chunk_id = chunk;
    new_used_block.offset = old_free_block.offset;
    new_used_block.size = size;
    used_blocks.push_back(new_used_block);
    // Update Old Free Block
    old_free_block.offset += size;
    old_free_block.size -= size;
    if (old_free_block.size == 0) {
        free_blocks.erase(it);
    }

    // Upload
    buffer->upload(new_used_block.offset, size, data);
    used_size += size;

    // Update Map
    chunk_to_offset[chunk] = new_used_block.offset;

    // Check Fragmentation
    check_fragmentation();
}

// Recreate/Defragment
void Storage::recreate(const ssize_t extra_size) {
    // Create New Buffer
    const ssize_t new_size = (used_size + extra_size) * 2;
    Buffer *new_buffer = new Buffer(new_size);

    // Copy Used Blocks
    intptr_t offset = 0;
    int i = 0;
    chunk_to_offset.clear();
    for (Block &block : used_blocks) {
        new_buffer->upload(offset, block.size, buffer->client_side_data + block.offset);
        block.offset = offset;
        chunk_to_offset[block.chunk_id] = offset;
        offset += block.size;
        i++;
    }

    // Update Free Blocks
    free_blocks.clear();
    Block block;
    block.offset = offset;
    block.size = new_size - offset;
    free_blocks.push_back(block);

    // Use New Buffer
    delete buffer;
    buffer = new_buffer;
    total_size = new_size;
}