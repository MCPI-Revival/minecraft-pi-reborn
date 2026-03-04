#undef _FILE_OFFSET_BITS
#undef _TIME_BITS
#define __USE_LARGEFILE64

#include <dirent.h>

// Minecraft: Pi Edition was not compiled with 64-bit filesystem support.
// This fixes readdir() to read directories properly.
MCPI_MODS_PUBLIC dirent *readdir(DIR *dirp) {
    // Call 64-Bit Function
    const dirent64 *original = readdir64(dirp);
    if (original == nullptr) {
        return nullptr;
    }
    // Copy Result Into 32-Bit Structure
    static dirent new_dirent;
    const char *ptr = original->d_name;
    for (char &c : new_dirent.d_name) {
        c = *ptr;
        ptr++;
    }
    new_dirent.d_type = original->d_type;
    return &new_dirent;
}
