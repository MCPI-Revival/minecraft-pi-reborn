#pragma once

#include <cstdint>
#include <type_traits>

#include <trampoline/types.h>

// Sanity Checks
#if __BYTE_ORDER != __LITTLE_ENDIAN
#error "Only Little Endian Is Supported"
#endif
static_assert(sizeof(int) == sizeof(int32_t));

// Utility Macros
#define block_pointer(T) static_assert(!std::is_pointer<T>::value, "Do Not Use Raw Pointers Here")
#define should_just_send_pointer() strcmp(getenv(MCPI_TRAMPOLINE_ENV), "syscall") == 0

// Alignment
#include "align.h"

// Include Inner Header
#if defined(MEDIA_LAYER_TRAMPOLINE_HOST)
#include "../host/host.h"
#elif defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
#include "../guest/guest.h"
#else
#error "Invalid Configuration"
#endif