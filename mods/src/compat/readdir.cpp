#undef _FILE_OFFSET_BITS
#define __USE_LARGEFILE64

#include <dirent.h>

// Minecraft: Pi Edition Was Not Compiled With 64-Bit Filesystem Support, So This Shims readdir() To Read Directories Properly

#define FILENAME_SIZE 256

dirent *readdir(DIR *dirp) {
    const dirent64 *original = readdir64(dirp);
    if (original == nullptr) {
        return nullptr;
    }
    static dirent new_dirent;
    for (int i = 0; i < FILENAME_SIZE; i++) {
        new_dirent.d_name[i] = original->d_name[i];
    }
    new_dirent.d_type = original->d_type;
    return &new_dirent;
}
