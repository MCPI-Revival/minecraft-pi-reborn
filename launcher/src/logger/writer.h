#pragma once

#include <string>
#include <fstream>

// Log Writer
struct LogWriter final {
    // Properties
    const std::string name;
    std::ofstream file;

    // Methods
    explicit LogWriter(const std::string &dir);
    ~LogWriter();
    void write(const unsigned char *data, std::streamsize size, bool assume_starts_outside_control_code);

private:
    // Control Code Stripper
    enum class State {
        NORMAL, // Not In A Control Code
        ESC, // Seen ESC (Possibly Inside Control Code)
        CSI, // Seen ESC[ (Definitely Inside Control Code)
    } state;
};