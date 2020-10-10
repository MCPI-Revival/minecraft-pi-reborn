#ifndef CXX_11_UTIL_H

#define CXX_11_UTIL

#ifdef __cplusplus
extern "C" {
#endif

#define CXX11_STRING_SIZE 24

struct cxx11_string {
    unsigned char data[CXX11_STRING_SIZE];
};

cxx11_string create_cxx11_string(const char *str);

#ifdef __cplusplus
}
#endif

#endif
