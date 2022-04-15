#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libreborn/libreborn.h>

#include "../home/home.h"
#include "../init/init.h"

void run_tests() {
    // Test ~/.minecraft-pi Permissions
    {
        char *path = home_get();
        int exists = access(path, F_OK) == 0;
        int can_write = exists ? access(path, R_OK | W_OK) == 0 : 1;

        if (!can_write) {
            // Failure
            ERR("Invalid Data Directory Permissions");
        }
    }
}
