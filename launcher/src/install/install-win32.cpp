#include <string>
#include <unistd.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <propvarutil.h>
#include <propkey.h>
#include <shellapi.h>

#include <libreborn/util/string.h>
#include <libreborn/util/io.h>
#include <libreborn/util/exec.h>
#include <libreborn/env/env.h>
#include <libreborn/log.h>
#include <libreborn/config.h>

#include "install.h"
#include "../util/util.h"

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
    static bool initialized = false;
    static std::wstring out;
    if (initialized) {
        return out;
    }
    initialized = true;
    // Add Suffix
    out = get_start_menu_path();
    if (!out.empty()) {
        const std::string name = reborn_config.app.id;
        const std::string suffix = path_separator + name + ".lnk";
        out += convert_utf8_to_wstring(suffix);
    }
    return out;
}
bool is_desktop_file_installed() {
    const std::wstring path = get_shortcut_path();
    return _waccess(path.c_str(), F_OK) == 0;
}

// Configure Relaunch Behavior
static std::pair<std::string, int> get_display_name_resource() {
    const std::string binary = get_binary();
    return {binary, 1};
}
void set_relaunch_env() {
    const std::pair<std::string, int> resource = get_display_name_resource();
    const std::string display_name_resource = '@' + resource.first + ",-" + safe_to_string(resource.second);
    set_and_print_env(_MCPI_RELAUNCH_DISPLAY_NAME_RESOURCE_ENV, display_name_resource.c_str());
    const char *cmd[] = {resource.first.c_str(), nullptr};
    const std::string cmd_str = make_cmd(cmd);
    set_and_print_env(_MCPI_RELAUNCH_COMMAND_ENV, cmd_str.c_str());
}

// Create Shortcut File
#define check(...) success = success && SUCCEEDED(__VA_ARGS__)
static bool create_link(const std::wstring &path) {
    IShellLinkW *link = nullptr;
    if (!SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&link)))) {
        return false;
    }

    // Get Strings
    const std::wstring description = convert_utf8_to_wstring(reborn_config.app.description);
    const std::wstring app_id = convert_utf8_to_wstring(reborn_config.app.id);
    const std::wstring binary = convert_utf8_to_wstring(get_binary());

    // Set Path & Description
    bool success = true;
    check(link->SetPath(binary.c_str()));
    check(link->SetDescription(description.c_str()));

    // Set App ID
    IPropertyStore *property_store = nullptr;
    check(link->QueryInterface(IID_PPV_ARGS(&property_store)));
    if (success) {
        PROPVARIANT prop_variant = {};
        check(InitPropVariantFromString(app_id.c_str(), &prop_variant));
        if (success) {
            check(property_store->SetValue(PKEY_AppUserModel_ID, prop_variant));
            const HRESULT clear_ret = PropVariantClear(&prop_variant);
            if (clear_ret != S_OK) {
                success = false;
            }
        }
        check(property_store->Commit());
        property_store->Release();
    }

    // Save Link
    IPersistFile *file = nullptr;
    check(link->QueryInterface(IID_PPV_ARGS(&file)));
    if (success) {
        check(file->Save(path.c_str(), TRUE));
        file->Release();
    }

    // Add Display Name
    const std::pair<std::string, int> resource = get_display_name_resource();
    const std::wstring resource_binary = convert_utf8_to_wstring(resource.first);
    check(SHSetLocalizedName(path.c_str(), resource_binary.c_str(), resource.second));

    // Return
    link->Release();
    return success;
}
#undef check
void copy_desktop_file() {
    const std::wstring path = get_shortcut_path();
    if (!path.empty() && init_com()) {
        // Create Link
        _wunlink(path.c_str());
        const bool success = create_link(path);
        // Free
        CoUninitialize();
        if (!success) {
            WARN("Unable To Create Start Menu Shortcut");
            _wunlink(path.c_str());
        } else {
            INFO("Created Shortcut: %ls", path.c_str());
        }
    }
}

// Uninstallation
void remove_desktop_file() {
    const std::wstring path = get_shortcut_path();
    if (!path.empty()) {
        const int ret = _wunlink(path.c_str());
        if (ret != 0 && errno != ENOENT) {
            ERR("Unable To Delete Shortcut: %ls: %s", path.c_str(), strerror(errno));
        }
        INFO("Deleted Shortcut: %ls", path.c_str());
    }
}