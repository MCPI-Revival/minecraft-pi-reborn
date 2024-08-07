cmake_minimum_required(VERSION 3.17.0)

# Avoid Warning About DOWNLOAD_EXTRACT_TIMESTAMP
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24.0)
    cmake_policy(SET CMP0135 NEW)
endif()

# Core Options
include(cmake/options/core-options.cmake)

# Build Mode
if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

# Start Project
project(minecraft-pi-reborn)

# Utility Functions
include(cmake/util/util.cmake)

# Sanity Checks
string(CONCAT ARM_SANITY_CHECK
    "include(CheckSymbolExists)\n"
    "check_symbol_exists(\"__arm__\" \"\" IS_ARM_TARGETING)"
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

# Extra Options
include(cmake/options/extra-options.cmake)

# Paths
include(cmake/options/paths.cmake)

# Required Compile Flags
string(CONCAT COMPILE_FLAGS_SETUP
    # Optimizations
    "if(CMAKE_BUILD_TYPE STREQUAL \"Release\")\n"
    "    add_compile_options(-O3)\n"
    "    add_link_options(-s)\n"
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
    "set(CMAKE_CXX_STANDARD 20)\n"

    # Skip RPath
    "set(CMAKE_SKIP_BUILD_RPATH TRUE)"
)
cmake_language(EVAL CODE "${COMPILE_FLAGS_SETUP}")

# Fast Math
add_compile_options(-ffast-math)

# Warnings
add_compile_options(-Wall -Wextra -Werror -Wpointer-arith -Wshadow -Wnull-dereference)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
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

# Install Prebuilt ARMHF Toolchain Sysroot
if(BUILD_NATIVE_COMPONENTS AND MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN)
    install_arm_sysroot()
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

# Packaging
if(BUILD_NATIVE_COMPONENTS)
    include(cmake/cpack/packaging.cmake)
endif()

# Superbuild
if(BUILD_NATIVE_COMPONENTS)
    include(ExternalProject)
    # Arguments
    set(ARM_OPTIONS "${MCPI_OPTIONS}")
    list(APPEND ARM_OPTIONS "-DMCPI_BUILD_MODE:STRING=arm")
    list(APPEND ARM_OPTIONS "-DCMAKE_INSTALL_MESSAGE:STRING=NEVER")
    list(APPEND ARM_OPTIONS "-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>")
    if(NOT MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN)
        if(DEFINED CMAKE_TOOLCHAIN_FILE)
            list(APPEND ARM_OPTIONS "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}")
        endif()
    else()
        list(APPEND ARM_OPTIONS "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${MCPI_CMAKE_TOOLCHAIN_FILE}")
    endif()
    list(APPEND ARM_OPTIONS "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}")
    # Build
    ExternalProject_Add(arm-components
        DOWNLOAD_COMMAND ""
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
        CMAKE_CACHE_ARGS ${ARM_OPTIONS}
        INSTALL_COMMAND
            "${CMAKE_COMMAND}" "-E"
            "rm" "-rf" "<INSTALL_DIR>/${MCPI_INSTALL_DIR}"
        COMMAND
            "${CMAKE_COMMAND}" "-E" "env"
            "DESTDIR="
            "${CMAKE_COMMAND}" "--install" "<BINARY_DIR>"
        USES_TERMINAL_CONFIGURE TRUE
        USES_TERMINAL_BUILD TRUE
        USES_TERMINAL_INSTALL TRUE
        BUILD_ALWAYS TRUE
    )
    # Install
    ExternalProject_Get_Property(arm-components INSTALL_DIR)
    install(DIRECTORY "${INSTALL_DIR}/${MCPI_INSTALL_DIR}/" DESTINATION "${MCPI_INSTALL_DIR}")
endif()
