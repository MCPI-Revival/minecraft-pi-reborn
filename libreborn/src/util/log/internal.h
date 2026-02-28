#pragma once

#include <optional>
#include <cstdio>

// Current Log File
extern std::optional<FILE *> log_file;
void close_log_file();

// Check If The Current Process Is The Logger
bool is_logger_process();