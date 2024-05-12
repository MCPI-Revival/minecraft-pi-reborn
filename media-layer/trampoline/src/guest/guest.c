#include <unistd.h>
#include <libreborn/libreborn.h>

#include "guest.h"

uint32_t _trampoline(uint32_t id, uint32_t *args) {
    // Make Syscall
    long ret = syscall(0x1337 /* See trampoline.patch */, id, args);
    if (ret == -1) {
        // Error
        ERR("Trampoline Error: %s", strerror(errno));
    }
    // Return
    return args[0];
}