# Specify Options
mcpi_option(OPEN_SOURCE_ONLY "Only Install Open-Source Code (Will Result In Broken Install)" BOOL FALSE)
mcpi_option(IS_APPIMAGE_BUILD "AppImage Build" BOOL FALSE)
mcpi_option(IS_FLATPAK_BUILD "Flatpak Build" BOOL FALSE)

# Server/Headless Builds
mcpi_option(SERVER_MODE "Server Mode" BOOL FALSE)
mcpi_option(HEADLESS_MODE "Headless Mode" BOOL "${MCPI_SERVER_MODE}")

# Prebuilt ARMHF Toolchain
if(BUILD_NATIVE_COMPONENTS)
    set(MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN FALSE)
    if(NOT IS_ARM_TARGETING)
        set(MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN TRUE)
    endif()
endif()

# Media Layer
if(NOT MCPI_HEADLESS_MODE)
    set(DEFAULT_USE_MEDIA_LAYER_PROXY FALSE)
    if(BUILD_NATIVE_COMPONENTS AND NOT IS_ARM_TARGETING)
        set(DEFAULT_USE_MEDIA_LAYER_PROXY TRUE)
    endif()
    mcpi_option(USE_MEDIA_LAYER_PROXY "Whether To Enable The Media Layer Proxy" BOOL "${DEFAULT_USE_MEDIA_LAYER_PROXY}")
    mcpi_option(USE_GLES1_COMPATIBILITY_LAYER "Whether To Enable The GLESv1_CM Compatibility Layer" BOOL TRUE)
else()
    set(MCPI_USE_MEDIA_LAYER_PROXY FALSE)
endif()
if(MCPI_USE_MEDIA_LAYER_PROXY)
    set(BUILD_MEDIA_LAYER_CORE "${BUILD_NATIVE_COMPONENTS}")
else()
    set(BUILD_MEDIA_LAYER_CORE "${BUILD_ARM_COMPONENTS}")
endif()

# Specify Variant Name
set(MCPI_VARIANT_NAME "minecraft-pi-reborn")
if(MCPI_SERVER_MODE)
    string(APPEND MCPI_VARIANT_NAME "-server")
else()
    string(APPEND MCPI_VARIANT_NAME "-client")
endif()

# App ID
set(DEFAULT_APP_ID "com.thebrokenrail.MCPIReborn")
if(MCPI_SERVER_MODE)
    string(APPEND DEFAULT_APP_ID "Server")
else()
    string(APPEND DEFAULT_APP_ID "Client")
endif()
set(MCPI_APP_ID "${DEFAULT_APP_ID}" CACHE STRING "App ID")

# App Title
mcpi_option(APP_BASE_TITLE "Base App Title" STRING "Minecraft: Pi Edition: Reborn")
set(DEFAULT_APP_TITLE "${MCPI_APP_BASE_TITLE}")
if(MCPI_SERVER_MODE)
    string(APPEND DEFAULT_APP_TITLE " (Server)")
else()
    string(APPEND DEFAULT_APP_TITLE " (Client)")
endif()
mcpi_option(APP_TITLE "App Title" STRING "${DEFAULT_APP_TITLE}")

# Skin Server
mcpi_option(SKIN_SERVER "Skin Server" STRING "https://raw.githubusercontent.com/MCPI-Revival/Skins/data")

# Paths
include("${CMAKE_CURRENT_LIST_DIR}/paths.cmake")
