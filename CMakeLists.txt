cmake_minimum_required(VERSION 3.25.0)

# Core Options
include(cmake/options/core-options.cmake)

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

# Utility Functions
include(cmake/util/util.cmake)
include(cmake/sdk/util.cmake)

# Build Mode
set(RELEASE_MODE "Release")
string(CONCAT BUILD_MODE_SETUP
    "get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)\n"
    "if(IS_MULTI_CONFIG)\n"
    "    force_set(CMAKE_CONFIGURATION_TYPES \"${RELEASE_MODE};Debug\" STRING)\n"
    "    set(FORCE_BUILD_TYPE \"\")\n"
    "elseif(NOT CMAKE_BUILD_TYPE)\n"
    "    set(FORCE_BUILD_TYPE \"${RELEASE_MODE}\")\n"
    "endif()\n"
    "if(DEFINED FORCE_BUILD_TYPE)\n"
    "    force_set(CMAKE_BUILD_TYPE \"\${FORCE_BUILD_TYPE}\" STRING)\n"
    "endif()\n"
)
cmake_language(EVAL CODE "${BUILD_MODE_SETUP}")

# Extra Options
include(cmake/options/extra-options.cmake)

# Paths
include(cmake/options/paths.cmake)

# Required Compile Flags
set(RELEASE_MODE_GENERATOR "\$<CONFIG:${RELEASE_MODE}>")
string(CONCAT COMPILE_FLAGS_SETUP
    # Optimizations
    "add_compile_options(\"\$<IF:${RELEASE_MODE_GENERATOR},-O3,-g>\")\n"
    "add_link_options(\"\$<${RELEASE_MODE_GENERATOR}:-s>\")\n"

    # Warnings
    "add_link_options(-Wl,--no-undefined)\n"

    # Default C Standard
    "add_definitions(-D_GNU_SOURCE)\n"
    "set(CMAKE_C_STANDARD 99)\n"
    "set(CMAKE_CXX_STANDARD 20)\n"

    # Build Option
    "force_set(CMAKE_POSITION_INDEPENDENT_CODE TRUE BOOL)\n"
    "force_set(CMAKE_SKIP_BUILD_RPATH TRUE BOOL)\n"
    "force_set(BUILD_SHARED_LIBS TRUE BOOL)\n"
)
if(BUILD_ARM_COMPONENTS)
    string(APPEND COMPILE_FLAGS_SETUP
        # Disable C++11 String ABI
        "add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=0)\n"
    )
endif()
cmake_language(EVAL CODE "${COMPILE_FLAGS_SETUP}")

# Build Dependencies
add_subdirectory(dependencies)

# Fast Math
add_compile_options(-ffast-math)

# Warnings
add_compile_options(
    -Wall
    -Wextra
    -Werror
    -Wpointer-arith
    -Wshadow
    -Wnull-dereference
    "$<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes>"
    "$<$<COMPILE_LANGUAGE:C>:-Wmissing-prototypes>"
    -Wmissing-declarations
)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    # Prevents False Positives
    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 10.0)
        add_compile_options(-Wno-stringop-overflow)
    endif()
    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 11.0)
        add_compile_options(-Wno-array-bounds -Wno-stringop-overread)
    endif()
endif()

# Build
add_subdirectory(libreborn)
add_subdirectory(media-layer)

# Switch Mode
if(BUILD_NATIVE_COMPONENTS)
    # Build
    add_subdirectory(launcher)
    add_subdirectory(images)

    # Install Prebuilt ARMHF Toolchain Sysroot
    if(MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN)
        install_arm_sysroot()
    endif()

    # Packaging
    include(cmake/cpack/packaging.cmake)

    # Superbuild
    include(cmake/superbuild.cmake)
elseif(BUILD_ARM_COMPONENTS)
    # Build
    add_subdirectory(symbols)
    add_subdirectory(mods)

    # SDK
    include(cmake/sdk/install.cmake)
endif()