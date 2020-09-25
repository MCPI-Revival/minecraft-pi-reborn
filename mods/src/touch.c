#include <libcore/libcore.h>

__attribute__((constructor)) static void init() {
    unsigned char patch_data[4] = {0x01, 0x00, 0x50, 0xe3};
    patch((void *) 0x292fc, patch_data);
    //unsigned char patch_data_2[4] = {0x00, 0x30, 0xa0, 0xe3};
    //patch((void *) 0x3d9b8, patch_data_2);
}
