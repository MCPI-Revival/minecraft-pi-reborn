# Pick Archive
set(toolchain_version "13.3.rel1")
execute_process(COMMAND uname -m OUTPUT_VARIABLE arch OUTPUT_STRIP_TRAILING_WHITESPACE)
if(arch STREQUAL "x86_64")
    set(toolchain_file "arm-gnu-toolchain-${toolchain_version}-x86_64-arm-none-linux-gnueabihf.tar.xz")
elseif(arch STREQUAL "aarch64" OR arch STREQUAL "armv8b" OR arch STREQUAL "armv8l")
    set(toolchain_file "arm-gnu-toolchain-${toolchain_version}-aarch64-arm-none-linux-gnueabihf.tar.xz")
else()
    message(FATAL_ERROR "Unable To Download Prebuilt ARMHF Toolchain")
endif()

# Download If Needed
include(FetchContent)
FetchContent_Declare(
    prebuilt-armhf-toolchain
    URL "${CMAKE_CURRENT_LIST_DIR}/../../archives/${toolchain_file}"
)
FetchContent_MakeAvailable(prebuilt-armhf-toolchain)
set(toolchain_dir "${prebuilt-armhf-toolchain_SOURCE_DIR}")

# Force Toolchain
file(WRITE "${toolchain_dir}/toolchain.cmake"
    "set(CMAKE_C_COMPILER \"\${CMAKE_CURRENT_LIST_DIR}/bin/arm-none-linux-gnueabihf-gcc\")\n"
    "set(CMAKE_CXX_COMPILER \"\${CMAKE_CURRENT_LIST_DIR}/bin/arm-none-linux-gnueabihf-g++\")\n"
    "set(CMAKE_SYSTEM_NAME \"Linux\")\n"
    "set(CMAKE_SYSTEM_PROCESSOR \"arm\")\n"
    "set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)\n"
)
force_set(MCPI_CMAKE_TOOLCHAIN_FILE "${toolchain_dir}/toolchain.cmake" FILEPATH)

# Build Sysroot
set(sysroot_dir "${CMAKE_CURRENT_BINARY_DIR}/bundled-armhf-sysroot")
if("${toolchain_dir}/bin/arm-none-linux-gnueabihf-gcc" IS_NEWER_THAN "${sysroot_dir}")
    # Create Directory
    file(REMOVE_RECURSE "${sysroot_dir}")
    file(MAKE_DIRECTORY "${sysroot_dir}")

    # Copy Files From Toolchain
    file(
        COPY "${toolchain_dir}/arm-none-linux-gnueabihf/libc/"
        DESTINATION "${sysroot_dir}"
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING
        PATTERN "*.so*"
    )

    # Delete Unneeded Files
    file(REMOVE_RECURSE "${sysroot_dir}/usr/lib/audit")
    file(REMOVE_RECURSE "${sysroot_dir}/usr/lib/gconv")

    # Strip Files
    if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
        file(GLOB_RECURSE files LIST_DIRECTORIES FALSE "${sysroot_dir}/*")
        foreach(file IN LISTS files)
            execute_process(
                COMMAND "${toolchain_dir}/bin/arm-none-linux-gnueabihf-strip" "${file}"
                RESULT_VARIABLE ret
                ERROR_QUIET
            )
            # Check Result
            if(NOT ret EQUAL 0)
                # Delete Invalid Files
                file(REMOVE "${file}")
            endif()
        endforeach()
    endif()
endif()

# Install Sysroot (Skipping Empty Directories)
function(install_arm_sysroot)
    file(GLOB_RECURSE files LIST_DIRECTORIES FALSE RELATIVE "${sysroot_dir}" "${sysroot_dir}/*")
    foreach(file IN LISTS files)
        cmake_path(GET file PARENT_PATH parent)
        install(PROGRAMS "${sysroot_dir}/${file}" DESTINATION "${MCPI_INSTALL_DIR}/sysroot/${parent}")
    endforeach()
endfunction()
