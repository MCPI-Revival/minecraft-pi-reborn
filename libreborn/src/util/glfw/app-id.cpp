#ifdef _WIN32
#include <shobjidl.h>
#include <propvarutil.h>
#include <propkey.h>

#include <libreborn/util/string.h>
#include <libreborn/util/io.h>
#include <libreborn/env/env.h>
#endif

#include "glfw.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

// Set ID Globally
void _reborn_set_app_id_global(const std::string &id) {
#ifndef _WIN32
    glfwWindowHintString(GLFW_X11_CLASS_NAME, id.c_str());
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, id.c_str());
#else
    const std::wstring wide_id = convert_utf8_to_wstring(id);
    (void) SetCurrentProcessExplicitAppUserModelID(wide_id.c_str());
#endif
}

// Windows-Specific Code
#ifdef _WIN32
// Set Window Property
static HRESULT set_property(IPropertyStore *property_store, const PROPERTYKEY &key, PROPVARIANT &value) {
    HRESULT ret = property_store->SetValue(key, value);
    const HRESULT clear_ret = PropVariantClear(&value);
    if (clear_ret != S_OK) {
        ret = clear_ret;
    }
    return ret;
}
static HRESULT set_property_str(IPropertyStore *property_store, const PROPERTYKEY &key, const std::string &value) {
    const std::wstring wide_value = convert_utf8_to_wstring(value);
    PROPVARIANT prop_variant = {};
    HRESULT ret = InitPropVariantFromString(wide_value.c_str(), &prop_variant);
    if (ret == S_OK) {
        ret = set_property(property_store, key, prop_variant);
    }
    return ret;
}
static HRESULT set_property_bool(IPropertyStore *property_store, const PROPERTYKEY &key, const bool value) {
    PROPVARIANT prop_variant = {};
    HRESULT ret = InitPropVariantFromBoolean(WINBOOL(value), &prop_variant);
    if (ret == S_OK) {
        ret = set_property(property_store, key, prop_variant);
    }
    return ret;
}
static HRESULT clear_property(IPropertyStore *property_store, const PROPERTYKEY &key) {
    PROPVARIANT prop_variant = {};
    PropVariantInit(&prop_variant);
    return property_store->SetValue(key, prop_variant);
}

// Set ID For A Specific Window
#define check(...) success = success && SUCCEEDED(__VA_ARGS__)
void _reborn_set_app_id_and_relaunch_behavior(GLFWwindow *window, const std::string &id) {
    const HWND native_window = glfwGetWin32Window(window);
    bool success = true;
    if (init_com()) {
        // Get Property Store For Window
        IPropertyStore *property_store = nullptr;
        check(SHGetPropertyStoreForWindow(native_window, IID_PPV_ARGS(&property_store)));
        if (success) {
            // Read Environment
            const char *relaunch_command = getenv(_MCPI_RELAUNCH_COMMAND_ENV);
            const char *relaunch_display_name_resource = getenv(_MCPI_RELAUNCH_DISPLAY_NAME_RESOURCE_ENV);
            const bool can_relaunch = relaunch_command && relaunch_display_name_resource;
            // Set Relaunch Command
            check(set_property_bool(property_store, PKEY_AppUserModel_PreventPinning, !can_relaunch));
            if (can_relaunch) {
                check(set_property_str(property_store, PKEY_AppUserModel_RelaunchCommand, relaunch_command));
                check(set_property_str(property_store, PKEY_AppUserModel_RelaunchDisplayNameResource, relaunch_display_name_resource));
            }
            // Set App ID
            check(set_property_str(property_store, PKEY_AppUserModel_ID, id));
            // Save
            check(property_store->Commit());
            property_store->Release();
        }
        CoUninitialize();
    }
    if (!success) {
        WARN("Unable To Set App ID");
    }
}

// Clear Window Settings
// See: https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shgetpropertystoreforwindow
void _reborn_free_window_properties(GLFWwindow *window) {
    const HWND native_window = glfwGetWin32Window(window);
    bool success = true;
#define check(...) success = success && SUCCEEDED(__VA_ARGS__)
    if (init_com()) {
        // Get Property Store For Window
        IPropertyStore *property_store = nullptr;
        check(SHGetPropertyStoreForWindow(native_window, IID_PPV_ARGS(&property_store)));
        if (success) {
            // Clear Values
            check(clear_property(property_store, PKEY_AppUserModel_PreventPinning));
            check(clear_property(property_store, PKEY_AppUserModel_RelaunchCommand));
            check(clear_property(property_store, PKEY_AppUserModel_RelaunchDisplayNameResource));
            check(clear_property(property_store, PKEY_AppUserModel_ID));
            // Save
            check(property_store->Commit());
            property_store->Release();
        }
        CoUninitialize();
    }
}
#undef check
#endif