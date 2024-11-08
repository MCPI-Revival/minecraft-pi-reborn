#include "extend-internal.h"

// VTable
SETUP_VTABLE(DynamicTexture)
    PATCH_VTABLE(tick);
}