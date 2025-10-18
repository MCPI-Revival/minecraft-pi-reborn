#include <fcntl.h>
#include <vector>
#include <unistd.h>

#include <libreborn/log.h>
#include <libreborn/util/string.h>

#include "logger.h"

// Determine File Name
static std::string get_filename(const std::string &dir) {
    // Get Timestamp
    const std::string time = format_time("%Y-%m-%d");

    // Get Log Filename
    std::string log_filename;
    int num = 1;
    do {
        std::string file = time + '-' + safe_to_string(num) + ".log";
        log_filename = dir + '/' + file;
        num++;
    } while (access(log_filename.c_str(), F_OK) != -1);

    // Return
    return log_filename;
}

// Constructor
LogWriter::LogWriter(const std::string &dir):
    name(get_filename(dir)),
    file(name, std::ios::binary),
    state(State::NORMAL)
{
    if (!file) {
        ERR("Unable To Create Log File: %s", name.c_str());
    }
}

// Destructor
LogWriter::~LogWriter() {
    file.close();
}

// Writer
void LogWriter::write(const unsigned char *data, const std::streamsize size, const bool assume_starts_outside_control_code) {
    // Detect And Strip Control Codes
    std::vector<unsigned char> output;
    output.reserve(size);
    if (assume_starts_outside_control_code) {
        // Assume The Current State Is Outside A Control Code
        state = State::NORMAL;
    }
    for (std::streamsize i = 0; i < size; i++) {
        static constexpr char ESC = '\x1b';
        unsigned char c = data[i];
        switch (state) {
            case State::NORMAL: {
                // Check If A Control Code Is Starting
                if (c == ESC) {
                    // ESC
                    state = State::ESC;
                } else {
                    // Normal Character
                    output.push_back(c);
                }
                break;
            }
            case State::ESC: {
                // Might Be Inside A Control Code
                if (c == '[') {
                    // Definitely Inside A Control Code
                    state = State::CSI;
                } else {
                    // Not A Control Code
                    // Return The ESC To The Output
                    output.push_back(ESC);
                    state = State::NORMAL;
                }
                break;
            }
            case State::CSI: {
                // Definitely Inside A Control Code
                if (c >= '@' && c <= '~') {
                    // End Of The Control Code
                    state = State::NORMAL;
                }
                break;
            }
        }
    }

    // Write
    file.write((const char *) output.data(), std::streamsize(output.size()));
}