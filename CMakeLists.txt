cmake_minimum_required(VERSION 3.13.0)

# Specify Options
option(MCPI_USE_MEDIA_LAYER_PROXY "Whether To Enable The Media Layer Proxy" FALSE)
option(MCPI_SERVER_MODE "Server Mode" FALSE)
option(MCPI_HEADLESS_MODE "Headless Mode" ${MCPI_SERVER_MODE})
set(MCPI_BUILD_MODE "both" CACHE STRING "\"arm\" = Build Only Code That Must Be ARM; \"native\" = Build Architecture-Independent Code; \"both\" = Build All Code As ARM")
set_property(CACHE MCPI_BUILD_MODE PROPERTY STRINGS "both" "arm" "native")
option(MCPI_OPEN_SOURCE_ONLY "Only Install Open-Source Code (Will Result In Broken Install)" FALSE)
option(MCPI_IS_APPIMAGE_BUILD "AppImage Build" FALSE)

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
    set(MCPI_VARIANT_NAME "${MCPI_VARIANT_NAME}-server")
else()
    set(MCPI_VARIANT_NAME "${MCPI_VARIANT_NAME}-client")
endif()

# Specify Installation Paths
set(MCPI_INSTALL_DIR "lib/${MCPI_VARIANT_NAME}")
set(MCPI_LIB_DIR "${MCPI_INSTALL_DIR}/lib")
set(MCPI_BIN_DIR "${MCPI_INSTALL_DIR}/bin")
set(MCPI_FALLBACK_LIB_DIR "${MCPI_INSTALL_DIR}/fallback-lib")

# Build Mode
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Start Project
project(minecraft-pi-reborn)

# Require ARM Compilation
if(USE_ARM32_TOOLCHAIN AND (NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "arm") AND (NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l"))
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
add_link_options(-Wl,--no-undefined)
add_definitions(-D_GNU_SOURCE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# Specify Constants
if(MCPI_SERVER_MODE)
    add_definitions(-DMCPI_SERVER_MODE)
endif()
if(MCPI_HEADLESS_MODE)
    add_definitions(-DMCPI_HEADLESS_MODE)
endif()
if(MCPI_IS_APPIMAGE_BUILD)
    add_definitions(-DMCPI_IS_APPIMAGE_BUILD)
endif()

# Version
file(STRINGS VERSION VERSION)
add_definitions(-DVERSION="${VERSION}")

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
