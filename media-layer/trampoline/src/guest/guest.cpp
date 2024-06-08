#include <unistd.h>
#include <libreborn/libreborn.h>
#include <string>
#include <sys/mman.h>

#include "guest.h"

// Syscall Method
static uint32_t trampoline_syscall(const uint32_t id, const uint32_t length, const unsigned char *args) {
    // Make Syscall
    long ret = syscall(TRAMPOLINE_SYSCALL, id, length, args);
    if (ret == -1) {
        // Error
        ERR("Trampoline Error: %s", strerror(errno));
    }
    // Return
    return *(uint32_t *) args;
}

// Pipe Method (Native Trampoline Only)
static int get_pipe(const char *env) {
    const char *value = getenv(env);
    if (value == nullptr) {
        IMPOSSIBLE();
    }
    std::string str = value;
    return std::stoi(str);
}
static uint32_t trampoline_pipe(const uint32_t id) {
    // Get Pipes
    static int arguments_pipe = -1;
    static int return_value_pipe = -1;
    if (arguments_pipe == -1) {
        arguments_pipe = get_pipe(TRAMPOLINE_ARGUMENTS_PIPE_ENV);
        return_value_pipe = get_pipe(TRAMPOLINE_RETURN_VALUE_PIPE_ENV);
    }
    // Write ID
    if (write(arguments_pipe, &id, sizeof(uint32_t)) != sizeof(uint32_t)) {
        ERR("Unable To Write Trampoline Command");
    }
    // Return
    uint32_t ret;
    if (read(return_value_pipe, &ret, sizeof(uint32_t)) != sizeof(uint32_t)) {
        ERR("Unable To Read Trampoline Return Value");
    }
    return ret;
}

// Main Function
uint32_t _raw_trampoline(const uint32_t id, const uint32_t length, const unsigned char *args) {
    // Configure Method
    static int use_syscall = -1;
    if (use_syscall == -1) {
        use_syscall = getenv(TRAMPOLINE_USE_PIPES_ENV) == nullptr;
    }
    // Use Correct Method
    if (use_syscall) {
        return trampoline_syscall(id, length, args);
    } else {
        return trampoline_pipe(id);
    }
}

// Arguments Memory
unsigned char *get_arguments_memory() {
    static unsigned char fallback[MAX_TRAMPOLINE_ARGS_SIZE];
    // Find Result
    static unsigned char *ret = nullptr;
    if (ret == nullptr) {
        ret = fallback;
        // Try Shared Memory
        const char *shared_memory_name = getenv(TRAMPOLINE_SHARED_MEMORY_ENV);
        if (shared_memory_name != nullptr) {
            int fd = shm_open(shared_memory_name, O_RDWR, 0600);
            if (fd == -1) {
                ERR("Unable To Open Shared Memory: %s", strerror(errno));
            }
            ret = (unsigned char *) mmap(nullptr, MAX_TRAMPOLINE_ARGS_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
            if (ret == MAP_FAILED) {
                ERR("Unable To Map Shared Memory: %s", strerror(errno));
            }
        }
    }
    // Return
    return ret;
}