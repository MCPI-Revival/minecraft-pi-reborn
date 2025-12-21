#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#endif

#include <libreborn/util/exec.h>
#include <libreborn/env/env.h>
#include <libreborn/config.h>
#include <libreborn/log.h>

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

// Download Timeout
static const char *get_timeout() {
    static bool loaded = false;
    static const char *out;
    if (!loaded) {
        loaded = true;
        // Get Value
        out = getenv(MCPI_DOWNLOAD_TIMEOUT_ENV);
        if (!out || strlen(out) == 0) {
            out = reborn_config.extra.download_timeout;
        }
        // Validate
        if (strcmp(out, "0") == 0 || out[0] == '-') {
            // Timeout Disabled
            out = nullptr;
        } else {
            for (const char *c = out; *c; c++) {
                if (!isdigit(*c)) {
                    ERR("Invalid Download Timeout: %s", out);
                }
            }
        }
    }
    return out;
}
__attribute__((constructor)) static void init_timeout() {
    get_timeout();
}

// Download From Internet
const std::vector<unsigned char> *download_from_internet(const std::string &dest, const std::string &url, const std::optional<std::string> &user_agent) {
    // Select Tool
#ifndef _WIN32
    // All Linux Distributions Have Wget
    const char *tool = "wget";
    const char *output_opt = "--output-document";
    const char *timeout_opt = "--timeout";
    const char *user_agent_opt = "--user-agent";
    const std::vector extra_opts = {
        // Disable Automatic Retries
        "--tries", "1"
    };
#else
    // Windows Only Has Curl
    const char *tool = "curl";
    const char *output_opt = "--output";
    const char *timeout_opt = "--max-time";
    const char *user_agent_opt = "--user-agent";
    const std::vector extra_opts = {
        // Follow Redirects
        "--location"
    };
#endif

    // Construct Command
    std::vector<const char *> command;
    command.push_back(tool);
    // Output
    command.push_back(output_opt);
    command.push_back(dest.c_str());
    // Timeout
    const char *timeout = get_timeout();
    if (timeout) {
        command.push_back(timeout_opt);
        command.push_back(timeout);
    }
    // User Agent
    if (user_agent.has_value()) {
        command.push_back(user_agent_opt);
        command.push_back(user_agent.value().c_str());
    }
    // Extra Options
    command.insert(command.end(), extra_opts.begin(), extra_opts.end());
    // URL
    command.push_back(url.c_str());
    // Terminate Command
    command.push_back(nullptr);

    // Run Command
    exit_status_t status = 0;
    const std::vector<unsigned char> *output = run_command(command.data(), &status);
    if (!is_exit_status_success(status)) {
        delete output;
        return nullptr;
    } else {
        return output;
    }
}