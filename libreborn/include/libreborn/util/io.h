#pragma once

#include <cstddef>

// Platform-Specific Constants
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
struct MCPI_REBORN_UTIL_PUBLIC Pipe {
    explicit Pipe();
    const HANDLE read;
    const HANDLE write;
};

// Lock File
MCPI_REBORN_UTIL_PUBLIC HANDLE lock_file(const char *file);
MCPI_REBORN_UTIL_PUBLIC void unlock_file(HANDLE fd);

// Safe write()
MCPI_REBORN_UTIL_PUBLIC void safe_write(int fd, const void *buf, size_t size);

// Initialize COM
#ifdef _WIN32
MCPI_REBORN_UTIL_PUBLIC bool init_com();
#endif