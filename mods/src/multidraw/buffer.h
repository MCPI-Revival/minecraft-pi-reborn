#pragma once

#include <cstdint>
#include <GLES/gl.h>

// Keep An OpenGL Buffer And Client-Side Memory Synchronized
struct Buffer {
    explicit Buffer(ssize_t size);
    ~Buffer();

    // Prevent Copying
    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    // Data
    GLuint server_side_data;
    unsigned char *client_side_data;

    // Upload Data
    void upload(intptr_t offset, ssize_t size, const void *data) const;
};