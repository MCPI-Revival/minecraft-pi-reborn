#include <cstring>

#include "buffer.h"

// Setup Buffer
Buffer::Buffer(const ssize_t size) {
    // Client-Side Data
    client_side_data = new unsigned char[size];
    // Server-Side Data
    server_side_data = 0;
    media_glGenBuffers(1, &server_side_data);
    media_glBindBuffer(GL_ARRAY_BUFFER, server_side_data);
    media_glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    clear_dirty();
}
Buffer::~Buffer() {
    // Client-Side Data
    delete[] client_side_data;
    // Server-Side Data
    media_glDeleteBuffers(1, &server_side_data);
}

// Upload Data
void Buffer::upload(const intptr_t offset, const ssize_t size, const void *data) {
    // Client-Side Data
    memcpy(client_side_data + offset, data, size);
    // Server-Side Data
    if (dirty_end == offset) {
        dirty_end += size;
    } else {
        sync();
        dirty_start = offset;
        dirty_end = offset + size;
    }
}

// "Dirty" Data
void Buffer::clear_dirty() {
    dirty_start = dirty_end = -1;
}
void Buffer::sync() {
    if (dirty_start >= 0) {
        media_glBindBuffer(GL_ARRAY_BUFFER, server_side_data);
        const void *data = client_side_data + dirty_start;
        media_glBufferSubData(GL_ARRAY_BUFFER, dirty_start, dirty_end - dirty_start, data);
    }
    clear_dirty();
}

