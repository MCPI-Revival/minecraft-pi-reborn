#pragma once

// Debian Bookworm Does Not Support std::format
// Always Use fmtlib
// Original: https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/common/alformat.hpp

#include <fmt/format.h>

namespace al {
    using fmt::format;
    using fmt::format_args;
    using fmt::format_string;
    using fmt::make_format_args;
    using fmt::string_view;
    using fmt::vformat;
}