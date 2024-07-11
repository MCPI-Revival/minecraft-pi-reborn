#include <cstring>

#include "buffer.h"

// Setup Buffer
Buffer::Buffer(const ssize_t size) {
    // Client-Side Data
    client_side_data = new unsigned char[size];
    // Server-Side Data
    server_side_data = 0;
    glGenBuffers(1, &server_side_data);
    glBindBuffer(GL_ARRAY_BUFFER, server_side_data);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}
Buffer::~Buffer() {
    // Client-Side Data
    delete[] client_side_data;
    // Server-Side Data
    glDeleteBuffers(1, &server_side_data);
}

// Upload Data
void Buffer::upload(const intptr_t offset, const ssize_t size, const void *data) const {
    // Client-Side Data
    memcpy(client_side_data + offset, data, size);
    // Server-Side Data
    glBindBuffer(GL_ARRAY_BUFFER, server_side_data);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}
