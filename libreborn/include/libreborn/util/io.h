#pragma once

#include <cstddef>

static constexpr char linux_path_separator = '/';
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define HANDLE_PRINTF "%p"
static constexpr char path_separator = '\\';
#else
typedef int HANDLE;
#define HANDLE_PRINTF "%i"
#define CloseHandle ::close
static constexpr char path_separator = linux_path_separator;
#endif

// Safe Version Of pipe()
struct Pipe {
    explicit Pipe(bool inheritable_on_windows);
    const HANDLE read;
    const HANDLE write;
};

// Lock File
HANDLE lock_file(const char *file);
void unlock_file(HANDLE fd);

// Safe write()
void safe_write(int fd, const void *buf, size_t size);