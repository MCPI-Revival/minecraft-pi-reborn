cmake_minimum_required(VERSION 3.16.0)

# Specify Options
option(MCPI_IS_MIXED_BUILD "Whether The Architecture-Independent And ARM Code Are Different Architecture" FALSE)
set(MCPI_BUILD_MODE "both" CACHE STRING "\"arm\" = Build Only Code That Must Be ARM; \"native\" = Build Architecture-Independent Code; \"both\" = Build All Code As ARM")
set_property(CACHE MCPI_BUILD_MODE PROPERTY STRINGS "both" "arm" "native")
option(MCPI_OPEN_SOURCE_ONLY "Only Install Open-Source Code (Will Result In Broken Install)" FALSE)
option(MCPI_IS_APPIMAGE_BUILD "AppImage Build" FALSE)
# Server/Headless Builds
option(MCPI_SERVER_MODE "Server Mode" FALSE)
option(MCPI_HEADLESS_MODE "Headless Mode" ${MCPI_SERVER_MODE})
# ARMHF Sysroot
option(MCPI_BUNDLE_ARMHF_SYSROOT "Whether To Include An ARMHF Sysroot" ${MCPI_IS_MIXED_BUILD})
set(MCPI_CUSTOM_BUNDLED_ARMHF_SYSROOT "" CACHE PATH "Custom Bundled ARMHF Sysroot")
# Media Layer
if(NOT MCPI_HEADLESS_MODE)
    option(MCPI_USE_MEDIA_LAYER_PROXY "Whether To Enable The Media Layer Proxy" ${MCPI_IS_MIXED_BUILD})
    option(MCPI_USE_GLES1_COMPATIBILITY_LAYER "Whether To Enable The GLESv1_CM Compatibility Layer" TRUE)
else()
    set(MCPI_USE_MEDIA_LAYER_PROXY FALSE)
    set(MCPI_USE_GLES1_COMPATIBILITY_LAYER FALSE)
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
set(DEFAULT_APP_TITLE "Minecraft: Pi Edition: Reborn")
if(MCPI_SERVER_MODE)
    string(APPEND DEFAULT_APP_TITLE " (Server)")
else()
    string(APPEND DEFAULT_APP_TITLE " (Client)")
endif()
set(MCPI_APP_TITLE "${DEFAULT_APP_TITLE}" CACHE STRING "App Title")

# Configure Build Mode
if(MCPI_BUILD_MODE STREQUAL "arm")
    set(USE_ARM32_TOOLCHAIN TRUE)
    set(BUILD_ARM_COMPONENTS TRUE)
    set(BUILD_NATIVE_COMPONENTS FALSE)
elseif(MCPI_BUILD_MODE STREQUAL "native")
    set(USE_ARM32_TOOLCHAIN FALSE)
    set(BUILD_ARM_COMPONENTS FALSE)
    set(BUILD_NATIVE_COMPONENTS TRUE)
elseif(MCPI_BUILD_MODE STREQUAL "both")
    set(USE_ARM32_TOOLCHAIN TRUE)
    set(BUILD_ARM_COMPONENTS TRUE)
    set(BUILD_NATIVE_COMPONENTS TRUE)
else()
    message(FATAL_ERROR "Invalid Mode")
endif()

# Utility Functions
include(cmake/util.cmake)

# Specify Variant Name
set(MCPI_VARIANT_NAME "minecraft-pi-reborn")
if(MCPI_SERVER_MODE)
    string(APPEND MCPI_VARIANT_NAME "-server")
else()
    string(APPEND MCPI_VARIANT_NAME "-client")
endif()

# Specify Installation Paths
set(MCPI_INSTALL_DIR "lib/${MCPI_VARIANT_NAME}")
set(MCPI_LIB_DIR "${MCPI_INSTALL_DIR}/lib")
set(MCPI_BIN_DIR "${MCPI_INSTALL_DIR}/bin")

# Build Mode
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Start Project
project(minecraft-pi-reborn)

# Sanity Check
if(BUILD_NATIVE_COMPONENTS AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^arm" AND NOT MCPI_IS_MIXED_BUILD)
    message(FATAL_ERROR "Project is configured as a mixed-buld, but MCPI_IS_MIXED_BUILD is disabled.")
endif()

# Require ARM Compilation
if(USE_ARM32_TOOLCHAIN AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    message(FATAL_ERROR "ARM-Targeting Compiler Required")
endif()

# Specify Default Installation Prefix
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "" FORCE)
endif()

# Optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3)
else()
    add_compile_options(-g)
endif()

# Use LLD When Using Clang
if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_link_options("-fuse-ld=lld")
endif()

# PIC
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# Fast Math
add_compile_options(-ffast-math)

# Buld Dependencies
add_subdirectory(dependencies)

# Warnings
add_compile_options(-Wall -Wextra -Werror -Wpointer-arith -Wshadow -Wnull-dereference)
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 10.0)
    # Prevents False Positives
    add_compile_options(-Wno-stringop-overflow)
endif()
add_link_options(-Wl,--no-undefined)
add_definitions(-D_GNU_SOURCE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

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
