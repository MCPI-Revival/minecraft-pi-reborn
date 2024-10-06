# Specify Options
mcpi_option(OPEN_SOURCE_ONLY "Only Install Open-Source Code (Will Result In Broken Install)" BOOL FALSE)
mcpi_option(IS_APPIMAGE_BUILD "AppImage Build" BOOL FALSE)
mcpi_option(IS_FLATPAK_BUILD "Flatpak Build" BOOL FALSE)
if(MCPI_IS_APPIMAGE_BUILD AND MCPI_IS_FLATPAK_BUILD)
    message(FATAL_ERROR "Invalid Build Configuration")
endif()

# Prebuilt ARMHF Toolchain
if(BUILD_NATIVE_COMPONENTS)
    set(MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN FALSE)
    if(NOT IS_ARM_TARGETING)
        set(MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN TRUE)
    endif()
    if(MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN)
        include("${CMAKE_CURRENT_LIST_DIR}/prebuilt-armhf-toolchain.cmake")
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

# Specify Variant Name
set(MCPI_VARIANT_NAME "minecraft-pi-reborn")

# App ID
mcpi_option(APP_ID "App ID" STRING "com.thebrokenrail.MCPIReborn")

# App Title
mcpi_option(APP_TITLE "App Title" STRING "Minecraft: Pi Edition: Reborn")

# Skin Server
mcpi_option(SKIN_SERVER "Skin Server" STRING "https://raw.githubusercontent.com/MCPI-Revival/Skins/data")

# Discord Invite URL
mcpi_option(DISCORD_INVITE "Discord Invite URL" STRING "https://discord.gg/mcpi-revival-740287937727561779")

# Version
set_property(
    DIRECTORY
    APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS VERSION
)
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/../../VERSION" MCPI_VERSION)
file(TIMESTAMP "${CMAKE_CURRENT_LIST_DIR}/../../VERSION" MCPI_VERSION_DATE "%Y-%m-%d" UTC)

# Documentation URL
mcpi_option(DOCUMENTATION "Documentation URL" STRING "https://gitea.thebrokenrail.com/minecraft-pi-reborn/minecraft-pi-reborn/src/tag/${MCPI_VERSION}/docs/")