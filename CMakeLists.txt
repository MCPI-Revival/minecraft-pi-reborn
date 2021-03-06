cmake_minimum_required(VERSION 3.13.0)

# Specify Options
option(MCPI_USE_MEDIA_LAYER_PROXY "Whether To Enable The Media Layer Proxy" FALSE)
option(MCPI_SERVER_MODE "Server Mode" FALSE)
set(MCPI_BUILD_MODE "both" CACHE STRING "\"arm\" = Build Only Code That Must Be ARM; \"native\" = Build Architecture-Independent Code; \"both\" = Build All Code As ARM")
set_property(CACHE MCPI_BUILD_MODE PROPERTY STRINGS "both" "arm" "native")

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

# Use Clang By Default
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(NOT DEFINED CMAKE_C_COMPILER)
        set(CMAKE_C_COMPILER clang)
    endif()
    if(NOT DEFINED CMAKE_CXX_COMPILER)
        set(CMAKE_CXX_COMPILER clang++)
    endif()
endif()

# Setup ARM Cross Compilation
if(USE_ARM32_TOOLCHAIN)
    include(cmake/arm-toolchain.cmake)
endif()

# Use LLD When Using Clang
if(CMAKE_C_COMPILER STREQUAL "clang")
    add_link_options("-fuse-ld=lld")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld") # Fix try_compile()
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
set(MCPI_INSTALL_DIR "opt/${MCPI_VARIANT_NAME}")
set(MCPI_LIB_DIR "${MCPI_INSTALL_DIR}/lib")
set(MCPI_FALLBACK_LIB_DIR "${MCPI_INSTALL_DIR}/fallback-lib")

# Optimizations
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3)
else()
    add_compile_options(-g)
    add_definitions(-DDEBUG)
endif()

# Start Project
project(minecraft-pi-reborn)

# Specify Default Installation Prefix
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/" CACHE PATH "" FORCE)
endif()

# Buld LibPNG + ZLib + Download Minecraft: Pi Edition
if(BUILD_ARM_COMPONENTS)
    add_subdirectory(dependencies)
endif()

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

# Build Mods
if(BUILD_ARM_COMPONENTS)
    add_subdirectory(mods)
endif()
