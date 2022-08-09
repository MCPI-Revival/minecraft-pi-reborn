# Locations
set(toolchain_dir "${CMAKE_CURRENT_BINARY_DIR}/prebuilt-armhf-toolchain")
set(sysroot_dir "${CMAKE_CURRENT_BINARY_DIR}/bundled-armhf-sysroot")

# Pick URL
execute_process(COMMAND uname -m OUTPUT_VARIABLE arch OUTPUT_STRIP_TRAILING_WHITESPACE)
if(arch STREQUAL "x86_64")
    set(toolchain_url "https://developer.arm.com/-/media/Files/downloads/gnu/11.3.rel1/binrel/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-linux-gnueabihf.tar.xz")
    set(toolchain_sha256 "3f76650b1d048036473b16b647b8fd005ffccd1a2869c10994967e0e49f26ac2")
elseif(arch STREQUAL "aarch64" OR arch STREQUAL "armv8b" OR arch STREQUAL "armv8l")
    set(toolchain_url "https://developer.arm.com/-/media/Files/downloads/gnu/11.3.rel1/binrel/arm-gnu-toolchain-11.3.rel1-aarch64-arm-none-linux-gnueabihf.tar.xz")
    set(toolchain_sha256 "2a12d92fcb7aadb059f42b9f18396c5d22ef76624225a855f4c28fa7fd888654")
else()
    message(FATAL_ERROR "Unable To Download Prebuilt ARMHF Toolchain")
endif()

# Download If Needed
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
    prebuilt-armhf-toolchain
    URL "${toolchain_url}"
    URL_HASH "SHA256=${toolchain_sha256}"
    SOURCE_DIR "${toolchain_dir}"
)
FetchContent_MakeAvailable(prebuilt-armhf-toolchain)
set(FETCHCONTENT_QUIET TRUE)

# Force Toolchain
file(WRITE "${toolchain_dir}/toolchain.cmake"
    "set(CMAKE_C_COMPILER \"\${CMAKE_CURRENT_LIST_DIR}/bin/arm-none-linux-gnueabihf-gcc\")\n"
    "set(CMAKE_CXX_COMPILER \"\${CMAKE_CURRENT_LIST_DIR}/bin/arm-none-linux-gnueabihf-g++\")\n"
    "set(CMAKE_SYSTEM_NAME \"Linux\")\n"
    "set(CMAKE_SYSTEM_PROCESSOR \"arm\")\n"
)
set(CMAKE_TOOLCHAIN_FILE "${toolchain_dir}/toolchain.cmake" CACHE STRING "" FORCE)

# Build Sysroot
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

    # Strip Files
    file(GLOB_RECURSE files LIST_DIRECTORIES FALSE "${sysroot_dir}/*")
    foreach(file IN LISTS files)
        execute_process(COMMAND "${toolchain_dir}/bin/arm-none-linux-gnueabihf-strip" "${file}" RESULT_VARIABLE ret)
        # Check Result
        if(NOT ret EQUAL 0)
            # Delete Invalid Files
            file(REMOVE "${file}")
        endif()
    endforeach()

    # Setup gconv
    file(
        COPY "${toolchain_dir}/arm-none-linux-gnueabihf/libc/usr/lib/gconv/gconv-modules"
        DESTINATION "${sysroot_dir}/usr/lib/gconv"
        USE_SOURCE_PERMISSIONS
    )
endif()

# Install Sysroot (Skipping Empty Directories)
file(GLOB_RECURSE files LIST_DIRECTORIES FALSE RELATIVE "${sysroot_dir}" "${sysroot_dir}/*")
foreach(file IN LISTS files)
    get_filename_component(parent "${file}" DIRECTORY)
    install(PROGRAMS "${sysroot_dir}/${file}" DESTINATION "${MCPI_INSTALL_DIR}/sysroot/${parent}")
endforeach()
