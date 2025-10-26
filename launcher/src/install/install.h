#pragma once

// Copying Desktop File
bool is_desktop_file_installed();
void copy_desktop_file();
#ifdef _WIN32
void set_relaunch_env();
#endif