# Only Include Open-Source Code
mcpi_option(OPEN_SOURCE_ONLY "Only Install Open-Source Code (Will Result In Broken Install)" BOOL FALSE)

# Packaging Type
set(no_packaging_type "none")
mcpi_option(PACKAGING_TYPE "Packaging Type" STRING "${no_packaging_type}")
set_property(CACHE MCPI_PACKAGING_TYPE PROPERTY STRINGS "${no_packaging_type}" "appimage" "flatpak")
macro(setup_packaging_type name)
    set(var "MCPI_IS_${name}_BUILD")
    string(TOLOWER "${name}" name)
    if(MCPI_PACKAGING_TYPE STREQUAL name)
        set("${var}" TRUE)
    else()
        set("${var}" FALSE)
    endif()
endmacro()
setup_packaging_type(APPIMAGE)
setup_packaging_type(FLATPAK)

# Prebuilt ARMHF Toolchain
if(BUILD_NATIVE_COMPONENTS)
    set(MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN FALSE)
    if(NOT IS_ARM_TARGETING)
        set(MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN TRUE)
    endif()
    if(MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN)
        include("${CMAKE_CURRENT_LIST_DIR}/../toolchain/prebuilt/armhf-toolchain.cmake")
    endif()
endif()

# Media Layer
set(DEFAULT_USE_MEDIA_LAYER_TRAMPOLINE FALSE)
if(BUILD_NATIVE_COMPONENTS AND NOT IS_ARM_TARGETING)
    set(DEFAULT_USE_MEDIA_LAYER_TRAMPOLINE TRUE)
endif()
mcpi_option(USE_MEDIA_LAYER_TRAMPOLINE "Whether To Enable The Media Layer Trampoline" BOOL "${DEFAULT_USE_MEDIA_LAYER_TRAMPOLINE}")
if(MCPI_USE_MEDIA_LAYER_TRAMPOLINE)
    set(BUILD_MEDIA_LAYER_CORE "${BUILD_NATIVE_COMPONENTS}")
else()
    set(BUILD_MEDIA_LAYER_CORE "${BUILD_ARM_COMPONENTS}")
endif()

# Author
mcpi_option(AUTHOR "Author" STRING "TheBrokenRail")
mcpi_option(AUTHOR_ID "Author ID" STRING "com.thebrokenrail")

# App Information
mcpi_option(APP_NAME "App Name" STRING "minecraft-pi-reborn")
mcpi_option(APP_ID "App ID" STRING "${MCPI_AUTHOR_ID}.MCPIReborn")
mcpi_option(APP_TITLE "App Title" STRING "Minecraft: Pi Edition: Reborn")

# Skin Server
mcpi_option(SKIN_SERVER "Skin Server" STRING "https://raw.githubusercontent.com/MCPI-Revival/Skins/data")

# Discord Invite URL
mcpi_option(DISCORD_INVITE "Discord Invite URL" STRING "https://discord.gg/mcpi-revival-740287937727561779")

# Version
set(version_file "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
set_property(
    DIRECTORY APPEND PROPERTY
    CMAKE_CONFIGURE_DEPENDS "${version_file}"
)
file(STRINGS "${version_file}" MCPI_VERSION)

# Homepage
mcpi_option(REPO_HOST "Repository Host" STRING "https://gitea.thebrokenrail.com")
mcpi_option(REPO_PATH "Repository Path" STRING "minecraft-pi-reborn/minecraft-pi-reborn")
mcpi_option(REPO "Repository URL" STRING "${MCPI_REPO_HOST}/${MCPI_REPO_PATH}")

# Documentation URL
mcpi_option(DOCS "Documentation URL" STRING "${MCPI_REPO}/src/tag/${MCPI_VERSION}/docs/")
mcpi_option(CHANGELOG "Changelog URL" STRING "${MCPI_REPO}/releases/tag/${MCPI_VERSION}")

# Packaging
set(MCPI_ARCH "unknown")
include(CheckSymbolExists)
function(check_arch symbol name)
    set(CMAKE_REQUIRED_QUIET TRUE)
    check_symbol_exists("${symbol}" "" "IS_ARCH_${name}")
    unset(CMAKE_REQUIRED_QUIET)
    if("${IS_ARCH_${name}}")
        set(MCPI_ARCH "${name}" PARENT_SCOPE)
    endif()
endfunction()
check_arch("__arm__" "armhf")
check_arch("__aarch64__" "arm64")
check_arch("__x86_64__" "amd64")
macro(get_package_file_name out version)
    set("${out}" "${MCPI_APP_NAME}-${version}-${MCPI_ARCH}")
endmacro()

# AppImage
if(MCPI_IS_APPIMAGE_BUILD)
    mcpi_option(APPIMAGE_EXT "AppImage Extension" STRING ".AppImage")
    mcpi_option(APPIMAGE_ZSYNC_EXT "AppImage Update Extension" STRING "${MCPI_APPIMAGE_EXT}.zsync")
    mcpi_option(APPIMAGE_JSON_URL "AppImage Update Checker URL" STRING "${MCPI_REPO_HOST}/api/v1/repos/${MCPI_REPO_PATH}/releases/latest")
    mcpi_option(APPIMAGE_VERSION_PLACEHOLDER "Version Placeholder In AppImage Download URL" STRING "%VERSION%")
    get_package_file_name(appimage_package_file_name "${MCPI_APPIMAGE_VERSION_PLACEHOLDER}")
    mcpi_option(APPIMAGE_DOWNLOAD_URL "AppImage Download URL" STRING "${MCPI_REPO}/releases/download/${MCPI_APPIMAGE_VERSION_PLACEHOLDER}/${appimage_package_file_name}${MCPI_APPIMAGE_EXT}")
endif()