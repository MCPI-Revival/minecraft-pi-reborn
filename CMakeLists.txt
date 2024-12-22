cmake_minimum_required(VERSION 3.25.0)

# Avoid Warning About DOWNLOAD_EXTRACT_TIMESTAMP
cmake_policy(SET CMP0135 NEW)

# Core Options
include(cmake/options/core-options.cmake)

# Utility Functions
include(cmake/util/util.cmake)

# Build Mode
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(IS_MULTI_CONFIG)
    force_set(CMAKE_CONFIGURATION_TYPES "Release;Debug" STRING)
    set(FORCE_BUILD_TYPE "")
elseif(NOT DEFINED CMAKE_BUILD_TYPE)
    set(FORCE_BUILD_TYPE "Release")
endif()
if(DEFINED FORCE_BUILD_TYPE)
    force_set(CMAKE_BUILD_TYPE "${FORCE_BUILD_TYPE}" STRING)
endif()

# Start Project
project(minecraft-pi-reborn)

# Sanity Checks
string(CONCAT ARM_SANITY_CHECK
    "include(CheckSymbolExists)\n"
    "check_symbol_exists(\"__arm__\" \"\" IS_ARM_TARGETING)\n"
)
if(BUILD_ARM_COMPONENTS)
    string(APPEND ARM_SANITY_CHECK
        "if(NOT IS_ARM_TARGETING)\n"
        "    message(FATAL_ERROR \"ARM-Targeting Compiler Required\")\n"
        "endif()\n"
    )
endif()
cmake_language(EVAL CODE "${ARM_SANITY_CHECK}")

# Extra Options
include(cmake/options/extra-options.cmake)

# Paths
include(cmake/options/paths.cmake)

# Required Compile Flags
set(RELEASE_MODE_GENERATOR "\$<CONFIG:Release>")
string(CONCAT COMPILE_FLAGS_SETUP
    # Optimizations
    "add_compile_options(\"\$<IF:${RELEASE_MODE_GENERATOR},-O3,-g>\")\n"
    "add_link_options(\"\$<${RELEASE_MODE_GENERATOR}:-s>\")\n"

    # PIC
    "set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)\n"

    # Warnings
    "add_link_options(-Wl,--no-undefined)\n"

    # C Standard
    "add_definitions(-D_GNU_SOURCE)\n"
    "set(CMAKE_C_STANDARD 99)\n"
    "set(CMAKE_CXX_STANDARD 20)\n"

    # Skip RPath
    "set(CMAKE_SKIP_BUILD_RPATH TRUE)\n"

    # Always Build Shared Libraries
    "set(BUILD_SHARED_LIBS TRUE CACHE BOOL \"\" FORCE)\n"
)
if(BUILD_ARM_COMPONENTS)
    string(APPEND COMPILE_FLAGS_SETUP
        # Disable C++11 String ABI
        "add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=0)\n"
    )
endif()
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

# Build Dependencies
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
    install(EXPORT sdk DESTINATION "${MCPI_SDK_DIR}" FILE "sdk-targets.cmake")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/sdk.cmake"
        # Sanity Check
        "${ARM_SANITY_CHECK}"
        # Compile Flags
        "${COMPILE_FLAGS_SETUP}"
        # Log
        "message(STATUS \"Using Reborn SDK v${MCPI_VERSION}\")\n"
        # Include Targets
        "include(\"\${CMAKE_CURRENT_LIST_DIR}/sdk-targets.cmake\")\n"
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sdk.cmake" DESTINATION "${MCPI_SDK_DIR}")
    # Calculate Hash Of SDK
    string(CONCAT SDK_HASH_SCRIPT
        # Prepare
        "set(dir \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${MCPI_SDK_DIR}\")\n"
        "set(out \"\${dir}/.hash\")\n"
        # Calculate Hashes
        "set(content \"\")\n"
        "file(GLOB_RECURSE files LIST_DIRECTORIES FALSE \"\${dir}/*\")\n"
        "foreach(file IN LISTS files)\n"
        "    file(SHA256 \"\${file}\" hash)\n"
        "    cmake_path(RELATIVE_PATH file BASE_DIRECTORY \"\${dir}\")\n"
        "    string(APPEND content \"\${hash} \${file}\\n\")\n"
        "endforeach()\n"
        # Write File
        "file(WRITE \"\${out}\" \"\${content}\")\n"
    )
    install(CODE "${SDK_HASH_SCRIPT}")
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
            set(ARM_TOOLCHAIN "${CMAKE_TOOLCHAIN_FILE}")
        endif()
    else()
        set(ARM_TOOLCHAIN "${MCPI_CMAKE_TOOLCHAIN_FILE}")
    endif()
    if(DEFINED ARM_TOOLCHAIN)
        list(APPEND ARM_OPTIONS "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${ARM_TOOLCHAIN}")
    endif()
    # Build
    ExternalProject_Add(arm-components
        # Source Directory
        DOWNLOAD_COMMAND ""
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
        # Configure
        CMAKE_CACHE_ARGS ${ARM_OPTIONS}
        CMAKE_GENERATOR "Ninja Multi-Config"
        # Build
        BUILD_COMMAND
            "${CMAKE_COMMAND}" "--build" "<BINARY_DIR>" "--config" "$<CONFIG>"
        # Install
        INSTALL_COMMAND
            "${CMAKE_COMMAND}" "-E"
            "rm" "-rf" "<INSTALL_DIR>/${MCPI_INSTALL_DIR}"
        COMMAND
            "${CMAKE_COMMAND}" "-E" "env"  "DESTDIR="
            "${CMAKE_COMMAND}" "--install" "<BINARY_DIR>" "--config" "$<CONFIG>"
        # Use Terminal
        USES_TERMINAL_CONFIGURE TRUE
        USES_TERMINAL_BUILD TRUE
        USES_TERMINAL_INSTALL TRUE
        # Always Build
        BUILD_ALWAYS TRUE
    )
    # Install
    ExternalProject_Get_Property(arm-components INSTALL_DIR)
    install(DIRECTORY "${INSTALL_DIR}/${MCPI_INSTALL_DIR}/" DESTINATION "${MCPI_INSTALL_DIR}")
endif()
