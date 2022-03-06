#include <stdlib.h>
#include <unistd.h>

#include "../bootstrap.h"

int main(int argc, char *argv[]) {
    // Set Home To Current Directory, So World Data Is Stored There
    char *launch_directory = getcwd(NULL, 0);
    setenv("HOME", launch_directory, 1);
    free(launch_directory);

    // Bootstrap
    bootstrap(argc, argv);
}
