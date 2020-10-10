// Use C++11 ABI
#undef _GLIBCXX_USE_CXX11_ABI
#define _GLIBCXX_USE_CXX11_ABI 1

#include <string>

#include "cxx11_util.h"

// Convert A C-String into A C++11 String That Can be Acessed In C++03 Code
cxx11_string create_cxx11_string(const char *str) {
    std::string *new_str = new std::string(str);
    int32_t new_size = sizeof (cxx11_string);
    int32_t old_size = sizeof *new_str;
    if (new_size != old_size) {
        fprintf(stderr, "Mismatched String Size: Expected: %i Real: %i\n", new_size, old_size);
    }
    return *reinterpret_cast<cxx11_string *>(new_str);
}
