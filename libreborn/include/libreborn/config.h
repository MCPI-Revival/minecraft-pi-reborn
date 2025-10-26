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
        APPIMAGE,
        FLATPAK,
        DEBIAN
    };
    PackagingType packaging;
    struct {
        const char *json_url;
        const char *version_placeholder;
        const char *download_url;
    } appimage;
};
extern const RebornConfig reborn_config;

// Fancy Version Name
std::string reborn_get_fancy_version();

// Runtime Configuration
bool reborn_is_headless();
bool reborn_is_server();