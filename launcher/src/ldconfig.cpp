#include <unistd.h>
#include <string>
#include <cstring>
#include <cstdio>

#include <libreborn/libreborn.h>

#include "ldconfig.h"

char *get_full_library_search_path() {
    std::string output;
    // Run
    FILE *file = popen("ldconfig -NXv 2> /dev/null", "r");
    // Read
    int running = 1;
    while (running) {
        char *line = NULL;
        size_t length = 0;
        if (getline(&line, &length, file) != -1) {
            // Convert to C++ String
            std::string str(line);
            // Remove Newline
            if (str.size() > 0 && str[str.size() - 1] == '\n') {
                str.pop_back();
            }
            // Interpret
            if (str.size() >= 2 && str[0] != '\t' && str[str.size() - 1] == ':') {
                output.append(str);
            }
        } else {
            running = 0;
        }
        free(line);
    }
    // Remove Colon
    if (output.size() > 0 && output[output.size() - 1] == ':') {
        output.pop_back();
    }
    // Close Process
    pclose(file);
    // Return
    char *output_str = strdup(output.c_str());
    ALLOC_CHECK(output_str);
    return output_str;
}
