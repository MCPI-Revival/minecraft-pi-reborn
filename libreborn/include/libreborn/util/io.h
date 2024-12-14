#pragma once

#include <cstddef>

// Safe Version Of pipe()
struct Pipe {
    Pipe();
    const int read;
    const int write;
};

// Lock File
int lock_file(const char *file);
void unlock_file(const char *file, int fd);

// Safe write()
void safe_write(int fd, const void *buf, size_t size);