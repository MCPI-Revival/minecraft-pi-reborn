#include <unistd.h>
#include <string>
#include <cstring>
#include <cstdio>

#include <sstream>

#include <libreborn/libreborn.h>

#include "ldconfig.h"

char *get_full_library_search_path() {
    std::string processed_output;
    // Run
    int return_code;
    const char *ldconfig_argv[] = {"/sbin/ldconfig", "-NXv", NULL};
    char *output = run_command(ldconfig_argv, &return_code);
    std::stringstream output_stream((std::string(output)));
    // Check Exit Code
    if (return_code != 0) {
        ERR("ldconfig Failed: Exit Code: %i", return_code);
    }

    // Read
    int running = 1;
    while (running) {
        std::string line;
        if (std::getline(output_stream, line)) {
            // Remove Newline
            if (line.size() > 0 && line[line.size() - 1] == '\n') {
                line.pop_back();
            }
            // Interpret
            if (line.size() >= 2 && line[0] != '\t' && line[line.size() - 1] == ':') {
                // Blacklist RPI Legacy GL Drivers
#define RPI_LEGACY_GL_PATH "/opt/vc"
                if (line.rfind(RPI_LEGACY_GL_PATH ":", 0) != 0 && line.rfind(RPI_LEGACY_GL_PATH "/", 0) != 0) {
                    processed_output.append(line);
                }
            }
        } else {
            running = 0;
        }
    }
    // Free Output
    free(output);

    // Remove Colon
    if (processed_output.size() > 0 && processed_output[processed_output.size() - 1] == ':') {
        processed_output.pop_back();
    }

    // Return
    char *output_str = strdup(processed_output.c_str());
    ALLOC_CHECK(output_str);
    return output_str;
}
