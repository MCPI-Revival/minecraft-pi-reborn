#pragma once

#if __BYTE_ORDER != __LITTLE_ENDIAN
#error "Only Little Endian Is Supported"
#endif

#if defined(MEDIA_LAYER_TRAMPOLINE_HOST)
#include "../host/host.h"
#elif defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
#include "../guest/guest.h"
#else
#error "Invalid Configuration"
#endif

//#define pun_to(type, x) (*(type *) &(x))
#define pun_to(type, x) \
    ({ \
        union { typeof(x) a; type b; } _pun; \
        _pun.a = x; \
        _pun.b; \
    })
