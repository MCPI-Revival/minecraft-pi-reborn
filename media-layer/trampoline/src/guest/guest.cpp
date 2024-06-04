#include <unistd.h>
#include <libreborn/libreborn.h>

#include "guest.h"

uint32_t _raw_trampoline(const uint32_t id, const uint32_t length, const unsigned char *args) {
    // Make Syscall
    long ret = syscall(0x1337 /* See trampoline.patch */, id, length, args);
    if (ret == -1) {
        // Error
        ERR("Trampoline Error: %s", strerror(errno));
    }
    // Return
    return *(uint32_t *) args;
}