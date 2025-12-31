#pragma once

// Original: https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/common/alformat.hpp

// Include Formatting Library
#define add(x) \
    namespace al { \
        using x::format; \
        using x::format_args; \
        using x::format_string; \
        using x::make_format_args; \
        using x::string_view; \
        using x::vformat; \
    }
#ifdef _WIN32
// The Windows WASAPI Backend Requires std::format
#include <format>
add(std)
#else
// Debian Bookworm Does Not Support std::format
#include <fmt/format.h>
add(fmt)
#endif
#undef add