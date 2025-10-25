#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#endif

#include <libreborn/util/exec.h>

#include "libreborn/log.h"

// Open URL
void open_url(const std::string &url) {
#ifndef _WIN32
    int return_code;
    const char *command[] = {"xdg-open", url.c_str(), nullptr};
    const std::vector<unsigned char> *output = run_command(command, &return_code);
    delete output;
    if (!is_exit_status_success(return_code)) {
        WARN("Unable To Open URL: %s", url.c_str());
    }
#else
    ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
#endif
}

// Download From Internet
const std::vector<unsigned char> *download_from_internet(const std::string &dest, const std::string &url) {
    exit_status_t status = 0;
    const char *const command[] = {
#ifndef _WIN32
        // All Linux Distributions Have Wget
        "wget", "-O",
#else
        // Windows Only Has Curl
        "curl", "-L", "-o",
#endif
        dest.c_str(), url.c_str(),
        nullptr
    };
    const std::vector<unsigned char> *output = run_command(command, &status);
    if (!is_exit_status_success(status)) {
        delete output;
        return nullptr;
    } else {
        return output;
    }
}