#pragma once

#include <string>

// Config
struct RebornConfig {
    // General
    struct {
        const char *version;
        const char *author;
        const char *arch;
    } general;

    // App Information
    struct {
        const char *title;
        const char *id;
        const char *name;
        const char *description;
    } app;

    // Extra Options
    struct {
        const char *skin_server;
        const char *discord_invite;
        const char *repo_url;
    } extra;

    // Documentation
    struct {
        const char *base;
        const char *getting_started;
        const char *changelog;
    } docs;

    // Internal
    struct {
        bool use_prebuilt_armhf_toolchain;
        const char *sdk_dir;
    } internal;

    // Game Information
    struct {
        const char *version;
        const char *download_url;
    } game;

    // Packaging
    enum class PackagingType {
        NONE,
        ZIP,
        APPIMAGE,
        FLATPAK,
        DEBIAN
    };
    PackagingType packaging;

    // Updater
    struct {
        const char *json_url;
        const char *version_placeholder;
        const char *web_url;
        const char *appimage_download_url;
    } updater;
};
extern const RebornConfig reborn_config;

// Fancy Version Name
std::string reborn_get_fancy_version();

// Check If A Package Manager Is Being Used
bool reborn_is_using_package_manager();

// Runtime Configuration
bool reborn_is_headless();
bool reborn_is_server();