#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>

#include <libreborn/libreborn.h>

#include "../init/init.h"

// Types
typedef long long time64_t; 
struct timespec64 {
    time64_t tv_sec;
    long tv_nsec;
};
typedef long int time32_t; 
struct timespec32 {
    time32_t tv_sec;
    long tv_nsec;
};

void run_tests() {
    // Test clock_gettime64
    {
        struct timespec64 ts64;
        long out = syscall(SYS_clock_gettime64, CLOCK_MONOTONIC, &ts64);
        if (out != 0) {
            if (errno == ENOSYS) {
                // clock_gettime64 Unsupported, Testing clock_gettime
                struct timespec32 ts32;
                out = syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &ts32);
                if (out != 0) {
                    // Failure
                    ERR("Unable To Run clock_gettime Syscall: %s", strerror(errno));
                }
            } else {
                // Failure
                ERR("Unable To Run clock_gettime64 Syscall: %s", strerror(errno));
            }
        }
    }

    // Test ~/.minecraft-pi Permissions
    {
        char *path = NULL;
        asprintf(&path, "%s/.minecraft-pi", getenv("HOME"));
        int ret = access(path, R_OK | W_OK);
        free(path);

        if (ret != 0) {
            // Failure
            ERR("%s", "Invalid ~/.minecraft-pi Permissions");
        }
    }
}