#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <libreborn/env/env.h>
#include <libreborn/log.h>
#include <libreborn/util/io.h>
#include <libreborn/util/exec.h>
#include <libreborn/config.h>

#include "../util/util.h"
#include "install.h"

// Paths
static std::string get_data_dir() {
    const char *xdg = getenv("XDG_DATA_HOME");
    if (xdg) {
        return xdg;
    } else {
        // Fallback
        const std::string home = require_env("HOME");
        return home + path_separator + ".local" + path_separator + "share";
    }
}
static std::string get_output_path(const std::string &path) {
    const std::string target = path_separator + std::string("share");
    size_t i = path.find(target);
    if (i == std::string::npos) {
        IMPOSSIBLE();
    }
    i += target.size();
    return get_data_dir() + path.substr(i);
}

// Comment Out Line In File
static void comment_line(const int fd, const std::string &target, const off_t offset) {
    // Go To Start
    lseek(fd, 0, SEEK_SET);
    // Find Target
    size_t pos = 0;
    while (true) {
        // Read Character
        char c;
        if (read(fd, &c, 1) != 1) {
            // End Of File
            return;
        }
        // Check
        const size_t tests[] = {pos, 0};
        for (const size_t test : tests) {
            if (c == target.at(test)) {
                pos = test + 1;
                break;
            }
        }
        if (pos == target.size()) {
            // Found Target
            break;
        }
    }
    // Patch
    lseek(fd, offset - off_t(target.size()), SEEK_CUR);
    constexpr char c = '#';
    safe_write(fd, &c, 1);
}
static void patch_line(const int fd, const std::string &key, const std::string &value) {
    // Comment Old Value
    const std::string target = key + '=';
    const std::string prefix = "\n";
    comment_line(fd, prefix + target, off_t(prefix.size()));
    // Write New Value
    lseek(fd, 0, SEEK_END);
    const std::string line = target + value + '\n';
    safe_write(fd, line.c_str(), line.size());
}

// Check Status
static bool is_file_installed(const std::string &path) {
    return access(get_output_path(path).c_str(), F_OK) == 0;
}
bool is_desktop_file_installed() {
    return is_file_installed(DESKTOP_FILE_PATH) && is_file_installed(ICON_PATH);
}

// Installation
static void install_file(const std::string &path) {
    const std::string binary_directory = get_binary_directory();
    const std::string output_path = get_output_path(path);
    // Create Directory
    std::string output_dir = output_path;
    chop_last_component(output_dir);
    make_directory(output_dir);
    // Copy File
    copy_file(binary_directory + path_separator + path, output_path, true);
}
static std::string get_exec() {
    return reborn_config.packaging == RebornConfig::PackagingType::APPIMAGE ? get_appimage_path() : get_binary();
}
static void update_desktop_database(std::string desktop_file) {
    chop_last_component(desktop_file);
    const char *const command[] = {"update-desktop-database", desktop_file.c_str(), nullptr};
    exit_status_t status = 0;
    const std::vector<unsigned char> *output = run_command(command, &status);
    delete output;
    if (is_exit_status_success(status)) {
        INFO("Updated Desktop Database");
    } else {
        WARN("Unable To Update Desktop Database");
    }
}
void copy_desktop_file() {
    // Copy Files
    install_file(DESKTOP_FILE_PATH);
    install_file(ICON_PATH);

    // Patch Desktop File
    const std::string desktop_file = get_output_path(DESKTOP_FILE_PATH);
    const int fd = open(desktop_file.c_str(), O_RDWR);
    if (fd < 0) {
        ERR("Unable To Open Desktop File: %s", strerror(errno));
    }
    patch_line(fd, "Icon", get_output_path(ICON_PATH));
    patch_line(fd, "Exec", get_exec());
    close(fd);

    // Invalidate Desktop Cache
    update_desktop_database(desktop_file);
}

// Uninstallation
static void uninstall_file(const std::string &path) {
    const std::string output_path = get_output_path(path);
    const int ret = unlink(output_path.c_str());
    if (ret != 0 && errno != ENOENT) {
        ERR("Unable To Uninstall File: %s: %s", output_path.c_str(), strerror(errno));
    }
    INFO("Uninstalled: %s", output_path.c_str());
}
void remove_desktop_file() {
    uninstall_file(DESKTOP_FILE_PATH);
    uninstall_file(ICON_PATH);
    update_desktop_database(get_output_path(DESKTOP_FILE_PATH));
}