#pragma once

#include <string>

std::string get_logs_folder();
void setup_logger();
void show_report(const char *log_filename);