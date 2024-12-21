# Target
set(target "arm-none-linux-gnueabihf")

# Pick Archive
set(toolchain_version "13.3.rel1")
execute_process(COMMAND uname -m OUTPUT_VARIABLE arch OUTPUT_STRIP_TRAILING_WHITESPACE)
if(arch STREQUAL "x86_64")
    set(toolchain_file "arm-gnu-toolchain-${toolchain_version}-x86_64-${target}.tar.xz")
elseif(arch STREQUAL "aarch64" OR arch STREQUAL "armv8b" OR arch STREQUAL "armv8l")
    set(toolchain_file "arm-gnu-toolchain-${toolchain_version}-aarch64-${target}.tar.xz")
else()
    message(FATAL_ERROR "Unable To Download Prebuilt ARMHF Toolchain")
endif()

# Download If Needed
include(FetchContent)
FetchContent_Declare(prebuilt-armhf-toolchain
    URL "${CMAKE_CURRENT_LIST_DIR}/../../archives/${toolchain_file}"
)
FetchContent_MakeAvailable(prebuilt-armhf-toolchain)
set(toolchain_dir "${prebuilt-armhf-toolchain_SOURCE_DIR}")

# Force Toolchain
file(WRITE "${toolchain_dir}/toolchain.cmake"
    "set(CMAKE_C_COMPILER \"\${CMAKE_CURRENT_LIST_DIR}/bin/${target}-gcc\")\n"
    "set(CMAKE_CXX_COMPILER \"\${CMAKE_CURRENT_LIST_DIR}/bin/${target}-g++\")\n"
    "set(CMAKE_SYSTEM_NAME \"Linux\")\n"
    "set(CMAKE_SYSTEM_PROCESSOR \"arm\")\n"
    "set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)\n"
)
force_set(MCPI_CMAKE_TOOLCHAIN_FILE "${toolchain_dir}/toolchain.cmake" FILEPATH)

# Build Sysroot
set(sysroot_dir "${CMAKE_CURRENT_BINARY_DIR}/bundled-armhf-sysroot")
set(sysroot_dir_debug "${sysroot_dir}/debug")
set(sysroot_dir_release "${sysroot_dir}/release")
if("${toolchain_dir}/bin/${target}-gcc" IS_NEWER_THAN "${sysroot_dir}")
    # Create Directory
    file(REMOVE_RECURSE "${sysroot_dir}")
    file(MAKE_DIRECTORY "${sysroot_dir_debug}")
    file(MAKE_DIRECTORY "${sysroot_dir_release}")

    # Copy Files From Toolchain
    file(
        COPY "${toolchain_dir}/${target}/libc/"
        DESTINATION "${sysroot_dir_debug}"
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING
        PATTERN "*.so*"
    )

    # Delete Unneeded Files
    file(REMOVE_RECURSE "${sysroot_dir_debug}/usr/lib/audit")
    file(REMOVE_RECURSE "${sysroot_dir_debug}/usr/lib/gconv")

    # Strip Files
    file(COPY "${sysroot_dir_debug}/." DESTINATION "${sysroot_dir_release}")
    file(GLOB_RECURSE files LIST_DIRECTORIES FALSE "${sysroot_dir_release}/*")
    foreach(file IN LISTS files)
        execute_process(
            COMMAND "${toolchain_dir}/bin/${target}-strip" "${file}"
            RESULT_VARIABLE ret
            ERROR_QUIET
        )
        # Delete Invalid Files
        if(NOT ret EQUAL 0)
            file(REMOVE "${file}")
        endif()
    endforeach()
endif()

# Install Sysroot (Skipping Empty Directories)
function(install_arm_sysroot_config config)
    set(dir "${sysroot_dir_${config}}")
    file(GLOB_RECURSE files LIST_DIRECTORIES FALSE RELATIVE "${dir}" "${dir}/*")
    foreach(file IN LISTS files)
        cmake_path(GET file PARENT_PATH parent)
        install(
            PROGRAMS "${dir}/${file}"
            DESTINATION "${MCPI_INSTALL_DIR}/sysroot/${parent}"
            CONFIGURATIONS "${config}"
        )
    endforeach()
endfunction()
function(install_arm_sysroot)
    install_arm_sysroot_config(debug)
    install_arm_sysroot_config(release)
endfunction()