cmake_minimum_required(VERSION 3.16.0)

# Avoid Warning About DOWNLOAD_EXTRACT_TIMESTAMP
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24.0)
    cmake_policy(SET CMP0135 NEW)
endif()

# Build Mode
set(MCPI_BUILD_MODE "native" CACHE STRING "\"arm\" = Build Only Code That Must Be ARM; \"native\" = Build Architecture-Independent Code")
set_property(CACHE MCPI_BUILD_MODE PROPERTY STRINGS "arm" "native")
if(MCPI_BUILD_MODE STREQUAL "arm")
    set(BUILD_ARM_COMPONENTS TRUE)
    set(BUILD_NATIVE_COMPONENTS FALSE)
elseif(MCPI_BUILD_MODE STREQUAL "native")
    set(BUILD_ARM_COMPONENTS FALSE)
    set(BUILD_NATIVE_COMPONENTS TRUE)
else()
    message(FATAL_ERROR "Invalid Mode")
endif()

# Specify Options
option(MCPI_IS_MIXED_BUILD "Whether The Architecture-Independent And ARM Code Are Different Architecture" FALSE)
option(MCPI_OPEN_SOURCE_ONLY "Only Install Open-Source Code (Will Result In Broken Install)" FALSE)
option(MCPI_IS_APPIMAGE_BUILD "AppImage Build" FALSE)

# Server/Headless Builds
option(MCPI_SERVER_MODE "Server Mode" FALSE)
option(MCPI_HEADLESS_MODE "Headless Mode" ${MCPI_SERVER_MODE})

# Media Layer
if(MCPI_HEADLESS_MODE)
    set(DEFAULT_USE_MEDIA_LAYER_PROXY FALSE)
else()
    set(DEFAULT_USE_MEDIA_LAYER_PROXY ${MCPI_IS_MIXED_BUILD})
endif()
option(MCPI_USE_MEDIA_LAYER_PROXY "Whether To Enable The Media Layer Proxy" ${DEFAULT_USE_MEDIA_LAYER_PROXY})
if(NOT MCPI_HEADLESS_MODE)
    option(MCPI_USE_GLES1_COMPATIBILITY_LAYER "Whether To Enable The GLESv1_CM Compatibility Layer" TRUE)
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
set(MCPI_APP_BASE_TITLE "Minecraft: Pi Edition: Reborn" CACHE STRING "Base App Title")
set(DEFAULT_APP_TITLE "${MCPI_APP_BASE_TITLE}")
if(MCPI_SERVER_MODE)
    string(APPEND DEFAULT_APP_TITLE " (Server)")
else()
    string(APPEND DEFAULT_APP_TITLE " (Client)")
endif()
set(MCPI_APP_TITLE "${DEFAULT_APP_TITLE}" CACHE STRING "App Title")

# Specify Variant Name
set(MCPI_VARIANT_NAME "minecraft-pi-reborn")
if(MCPI_SERVER_MODE)
    string(APPEND MCPI_VARIANT_NAME "-server")
else()
    string(APPEND MCPI_VARIANT_NAME "-client")
endif()

# Skin Server
set(MCPI_SKIN_SERVER "https://raw.githubusercontent.com/MCPI-Revival/Skins/data" CACHE STRING "Skin Server")

# Specify Installation Paths
set(MCPI_INSTALL_DIR "lib/${MCPI_VARIANT_NAME}")
set(MCPI_BIN_DIR "${MCPI_INSTALL_DIR}/bin")
set(MCPI_LEGAL_DIR "${MCPI_INSTALL_DIR}/legal") # For Software Licenses
set(MCPI_SDK_DIR "${MCPI_INSTALL_DIR}/sdk")
set(MCPI_SDK_LIB_DIR "${MCPI_SDK_DIR}/lib")
set(MCPI_SDK_INCLUDE_DIR "${MCPI_SDK_DIR}/include")

# Library Directory
set(MCPI_LIB_DIR "${MCPI_INSTALL_DIR}/lib")
if(BUILD_ARM_COMPONENTS)
    string(APPEND MCPI_LIB_DIR "/arm")
elseif(BUILD_NATIVE_COMPONENTS)
    string(APPEND MCPI_LIB_DIR "/native")
endif()

# Share Directory
set(MCPI_SHARE_DIR "share")
if(MCPI_IS_APPIMAGE_BUILD)
    string(PREPEND MCPI_SHARE_DIR "usr/")
endif()

# Build Mode
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Prebuilt ARMHF Toolchain
option(MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN "Whether To Use A Prebuilt ARMHF Toolchain For Building ARM Components" ${MCPI_IS_MIXED_BUILD})
if(BUILD_ARM_COMPONENTS AND MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN)
    include(cmake/prebuilt-armhf-toolchain.cmake)
endif()

# Start Project
project(minecraft-pi-reborn)

# Utility Functions
include(cmake/util.cmake)

# Sanity Checks
string(CONCAT ARM_SANITY_CHECK
    "set(IS_ARM_TARGETING FALSE)\n"
    "if(CMAKE_SYSTEM_PROCESSOR STREQUAL \"arm\" OR CMAKE_SYSTEM_PROCESSOR STREQUAL \"armv7l\")\n"
    "    set(IS_ARM_TARGETING TRUE)\n"
    "endif()"
)
if(BUILD_ARM_COMPONENTS)
    string(CONCAT ARM_SANITY_CHECK
        "${ARM_SANITY_CHECK}\n"
        "if(NOT IS_ARM_TARGETING)\n"
        "    message(FATAL_ERROR \"ARM-Targeting Compiler Required\")\n"
        "endif()"
    )
endif()
cmake_language(EVAL CODE "${ARM_SANITY_CHECK}")
if(BUILD_NATIVE_COMPONENTS AND NOT IS_ARM_TARGETING AND NOT MCPI_IS_MIXED_BUILD)
    message(FATAL_ERROR "Project is configured as a mixed-buld, but MCPI_IS_MIXED_BUILD is disabled.")
endif()

# Specify Default Installation Prefix
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(DEFAULT_PREFIX "/usr")
    if(MCPI_IS_APPIMAGE_BUILD)
        set(DEFAULT_PREFIX "/")
    endif()
    set(CMAKE_INSTALL_PREFIX "${DEFAULT_PREFIX}" CACHE PATH "" FORCE)
    set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT FALSE)
endif()

# Required Compile Flags
string(CONCAT COMPILE_FLAGS_SETUP
    # Optimizations
    "if(CMAKE_BUILD_TYPE STREQUAL \"Release\")\n"
    "    add_compile_options(-O3 -s)\n"
    "else()\n"
    "    add_compile_options(-g)\n"
    "endif()\n"

    # PIC
    "set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)\n"

    # Warnings
    "add_link_options(-Wl,--no-undefined)\n"

    # C Standard
    "add_definitions(-D_GNU_SOURCE)\n"
    "set(CMAKE_C_STANDARD 99)\n"
    "set(CMAKE_CXX_STANDARD 11)\n"

    # Skip RPath
    "set(CMAKE_SKIP_BUILD_RPATH TRUE)"
)
cmake_language(EVAL CODE "${COMPILE_FLAGS_SETUP}")

# Fast Math
add_compile_options(-ffast-math)

# Warnings
add_compile_options(-Wall -Wextra -Werror -Wpointer-arith -Wshadow -Wnull-dereference)
if(CMAKE_C_COMPILER_ID STREQUAL \"GNU\")
    # Prevents False Positives
    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 10.0)
        add_compile_options(-Wno-stringop-overflow)
    endif()
    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 11.0)
        add_compile_options(-Wno-array-bounds -Wno-stringop-overread)
    endif()
endif()

# Buld Dependencies
add_subdirectory(dependencies)

# Version
set_property(
    DIRECTORY
    APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS VERSION
)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" MCPI_VERSION)
file(TIMESTAMP "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" MCPI_VERSION_DATE "%Y-%m-%d" UTC)

# Build libreborn
add_subdirectory(libreborn)

# Build Media Layer
add_subdirectory(media-layer)

# Build Launcher
if(BUILD_NATIVE_COMPONENTS)
    add_subdirectory(launcher)
endif()

# Include Symbols
if(BUILD_ARM_COMPONENTS)
    add_subdirectory(symbols)
endif()

# Build Mods
if(BUILD_ARM_COMPONENTS)
    add_subdirectory(mods)
endif()

# Include Images
if(BUILD_NATIVE_COMPONENTS)
    add_subdirectory(images)
endif()

# Install SDK
if(BUILD_ARM_COMPONENTS)
    install(EXPORT sdk DESTINATION "${MCPI_SDK_DIR}" FILE "sdk-targets.cmake" EXPORT_LINK_INTERFACE_LIBRARIES)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/sdk.cmake"
        # Compile Flags
        "${COMPILE_FLAGS_SETUP}\n"
        # Snaity Check
        "${ARM_SANITY_CHECK}\n"
        # Log
        "message(STATUS \"Using Reborn SDK v${MCPI_VERSION}\")\n"
        # Include Targets
        "include(\"\${CMAKE_CURRENT_LIST_DIR}/sdk-targets.cmake\")\n"
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sdk.cmake" DESTINATION "${MCPI_SDK_DIR}")
endif()
