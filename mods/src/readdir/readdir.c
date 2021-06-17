#define __USE_LARGEFILE64

#include <stddef.h>
#include <stdint.h>
#include <dirent.h>

// Minecraft: Pi Edition Was Not Compiled With 64-Bit Filesystem Support, So This Shims readdir() To Read Directories Properly

#define FILENAME_SIZE 256

struct dirent *readdir(DIR *dirp) {
    struct dirent64 *original = readdir64(dirp);
    if (original == NULL) {
        return NULL;
    }
    static struct dirent new;
    for (int i = 0; i < FILENAME_SIZE; i++) {
        new.d_name[i] = original->d_name[i];
    }
    new.d_type = original->d_type;
    return &new;
}
