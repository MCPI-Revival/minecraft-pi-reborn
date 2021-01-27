#include <libreborn/libreborn.h>

#include "../feature/feature.h"
#include "misc.h"
#include "../init/init.h"

#include "../minecraft.h"

void init_misc() {
    if (feature_has("Remove Invalid Item Background")) {
        // Remove Invalid Item Background (A Red Background That Appears For Items That Are Not Included In The gui_blocks Atlas)
        unsigned char invalid_item_background_patch[4] = {0x00, 0xf0, 0x20, 0xe3};
        patch((void *) 0x63c98, invalid_item_background_patch);
    }

    // Init C++
    init_misc_cpp();
}