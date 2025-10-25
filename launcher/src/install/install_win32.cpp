#include <string>
#include <unistd.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>

#include <libreborn/log.h>
#include <libreborn/config.h>

#include "install.h"
#include "libreborn/util/io.h"

// Utility Functions
static bool init_com() {
    const HRESULT ret = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    const bool success = ret == S_OK;
    if (!success) {
        WARN("Unable To Initialize COM: %ld", ret);
    }
    return success;
}
static std::wstring convert_utf8_to_wstring(const std::string &str) {
    std::wstring out;
    const int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (size > 0) {
        out.resize(size - 1, '\0');
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, out.data(), size);
    }
    return out;
}

// Check If Shortcut Was Installed
static std::wstring get_start_menu_path() {
    std::wstring out;
    // Get Path
    if (init_com()) {
        PWSTR path = nullptr;
        const HRESULT ret1 = SHGetKnownFolderPath(FOLDERID_Programs, KF_FLAG_CREATE | KF_FLAG_INIT, nullptr, &path);
        if (ret1 != S_OK) {
            WARN("Unable To Get Start Menu Folder Folder: %ld", ret1);
        } else {
            out = path;
        }
        // Free
        CoTaskMemFree(path);
        CoUninitialize();
    }
    // Return
    return out;
}
static std::wstring get_shortcut_path() {
    std::wstring out = get_start_menu_path();
    // Add Suffix
    if (!out.empty()) {
        const std::string suffix = path_separator + std::string(reborn_config.app.name) + ".lnk";
        out += convert_utf8_to_wstring(suffix);
    }
    return out;
}
bool is_desktop_file_installed() {
    const std::wstring path = get_shortcut_path();
    return _waccess(path.c_str(), F_OK) == 0;
}

// Create Shortcut File
void copy_desktop_file() {
    const std::wstring path = get_shortcut_path();
    if (!path.empty() && init_com()) {
        bool success = false;
        // Create Link
        IShellLinkW *link;
        HRESULT ret1 = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID *) &link);
        if (ret1 != S_OK) {
            bool ret2 = link->SetPath(path.c_str()) == S_OK;
            ret2 = ret2 && link->SetDescription(convert_utf8_to_wstring(reborn_config.app.title).c_str()) == S_OK;
            if (ret2) {
                // Copy Link
                IPersistFile *file;
                ret1 = link->QueryInterface(IID_IPersistFile, (LPVOID *) &file);
                if (ret1 == S_OK) {
                    success = file->Save(path.c_str(), TRUE) == S_OK;
                    file->Release();
                }
            }
        }
        // Free
        CoUninitialize();
        if (!success) {
            WARN("Unable To Create Start Menu Shortcut");
        }
    }
}