#include <unistd.h>
#include <libreborn/libreborn.h>
#include <string>

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

// Pipe Method
#ifdef MCPI_USE_NATIVE_TRAMPOLINE
static int get_pipe(const char *env) {
    const char *value = getenv(env);
    if (value == nullptr) {
        IMPOSSIBLE();
    }
    std::string str = value;
    return std::stoi(str);
}
static uint32_t trampoline_pipe(const uint32_t id, const uint32_t length, const unsigned char *args) {
    // Get Pipes
    static int arguments_pipe = -1;
    static int return_value_pipe = -1;
    if (arguments_pipe == -1) {
        arguments_pipe = get_pipe(TRAMPOLINE_ARGUMENTS_PIPE_ENV);
        return_value_pipe = get_pipe(TRAMPOLINE_RETURN_VALUE_PIPE_ENV);
    }
    // Write Arguments
    trampoline_pipe_arguments data = {
        .id = id,
        .length = length,
        .args_addr = uint32_t(args)
    };
    if (write(arguments_pipe, &data, sizeof(trampoline_pipe_arguments)) != sizeof(trampoline_pipe_arguments)) {
        ERR("Unable To Write Trampoline Command");
    }
    // Return
    uint32_t ret;
    if (read(return_value_pipe, &ret, sizeof(uint32_t)) != sizeof(uint32_t)) {
        ERR("Unable To Read Trampoline Return Value");
    }
    return ret;
}
#else
static uint32_t trampoline_pipe(__attribute__((unused)) const uint32_t id, __attribute__((unused)) const uint32_t length, __attribute__((unused)) const unsigned char *args) {
    IMPOSSIBLE();
}
#endif

// Main Function
uint32_t _raw_trampoline(const uint32_t id, const uint32_t length, const unsigned char *args) {
    // Configure Method
    static int use_syscall = -1;
    if (use_syscall == -1) {
        use_syscall =
#ifdef MCPI_USE_NATIVE_TRAMPOLINE
            getenv(TRAMPOLINE_USE_PTRACE_ENV) != nullptr
#else
            1
#endif
            ;
    }
    // Use Correct Method
    if (use_syscall) {
        return trampoline_syscall(id, length, args);
    } else {
        return trampoline_pipe(id, length, args);
    }
}