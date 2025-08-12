#include <algorithm>
#include <ctime>
#include <cstring>

#include <libreborn/util/string.h>
#include <libreborn/log.h>

// Sanitize String
void sanitize_string(std::string &str, const int max_length, const bool allow_newlines) {
    // Truncate Message
    if (max_length >= 0 && str.size() > size_t(max_length)) {
        str = str.substr(0, max_length);
    }
    // Loop Through Message
    if (!allow_newlines) {
        for (char &x : str) {
            if (x == '\n') {
                // Replace Newline
                x = ' ';
            }
        }
    }
}

// Format Time
static std::string _format_time(const char *fmt, const time_t raw_time) {
    const tm *time_info = localtime(&raw_time);
    if (time_info == nullptr) {
        ERR("Unable To Determine Current Time: %s", strerror(errno));
    }
    char buf[512];
    strftime(buf, sizeof(buf), fmt, time_info);
    return std::string(buf);
}
std::string format_time(const char *fmt) {
    time_t raw_time;
    time(&raw_time);
    return _format_time(fmt, raw_time);
}
std::string format_time(const char *fmt, const int time) {
    // This Will Break In 2038
    return _format_time(fmt, time);
}

// Trim String
static void ltrim(std::string &s) {
    s.erase(s.begin(), std::ranges::find_if(s, [](const unsigned char ch) {
        return !std::isspace(ch);
    }));
}
static void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](const unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}
void trim(std::string &str) {
    rtrim(str);
    ltrim(str);
}

// Safe std::to_string
// Because C++26 Changed It
// https://www.sandordargo.com/blog/2025/07/09/cpp26-format-part-1
#define def(type, format) \
    std::string safe_to_string(const type x) { \
        char buf[64]; \
        sprintf(buf, format, x); \
        return buf; \
    }
def(float, "%f")
def(int, "%d")
def(size_t, "%zu")
#undef def
