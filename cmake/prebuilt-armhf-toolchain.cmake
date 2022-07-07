# Locations
set(toolchain_dir "${CMAKE_CURRENT_SOURCE_DIR}/.prebuilt-armhf-toolchain")
set(sysroot_dir "${CMAKE_CURRENT_BINARY_DIR}/bundled-armhf-sysroot")

# Force Toolchain
set(CMAKE_C_COMPILER "${toolchain_dir}/bin/arm-none-linux-gnueabihf-gcc")
set(CMAKE_CXX_COMPILER "${toolchain_dir}/bin/arm-none-linux-gnueabihf-g++")
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "arm")
unset(CMAKE_TOOLCHAIN_FILE CACHE)

# Download If Needed
if(NOT EXISTS "${CMAKE_C_COMPILER}")
    # Pick URL
    execute_process(COMMAND uname -m OUTPUT_VARIABLE arch OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(arch STREQUAL "x86_64")
        set(toolchain_url "https://developer.arm.com/-/media/Files/downloads/gnu/11.2-2022.02/binrel/gcc-arm-11.2-2022.02-x86_64-arm-none-linux-gnueabihf.tar.xz")
        set(toolchain_sha256 "c254f7199261fe76c32ef42187502839bda7efad0a66646cf739d074eff45fad")
    elseif(arch STREQUAL "aarch64_be" OR arch STREQUAL "aarch64" OR arch STREQUAL "armv8b" OR arch STREQUAL "armv8l")
        set(toolchain_url "https://developer.arm.com/-/media/Files/downloads/gnu/11.2-2022.02/binrel/gcc-arm-11.2-2022.02-aarch64-arm-none-linux-gnueabihf.tar.xz")
        set(toolchain_sha256 "c5603772af016008ddacb7e475dc226d0cccdf069925dfded43e452a59774fc3")
    else()
        message(FATAL_ERROR "Unable To Download Prebuilt ARMHF Toolchain")
    endif()

    # Download
    message(STATUS "Downloading Prebuilt ARMHF Toolchain...")
    file(REMOVE_RECURSE "${toolchain_dir}")
    include(FetchContent)
    set(FETCHCONTENT_QUIET FALSE)
    FetchContent_Declare(
        prebuilt-armhf-toolchain
        URL "${toolchain_url}"
        URL_HASH "SHA256=${toolchain_sha256}"
        SOURCE_DIR "${toolchain_dir}"
    )
    FetchContent_Populate(prebuilt-armhf-toolchain)
    # Force Sysroot Rebuild
    file(REMOVE_RECURSE "${sysroot_dir}")
endif()

# Build Sysroot
if(NOT EXISTS "${sysroot_dir}")
    # Create Directory
    file(MAKE_DIRECTORY "${sysroot_dir}")

    # Copy Files From Toolchain
    file(
        COPY "${toolchain_dir}/arm-none-linux-gnueabihf/libc/"
        DESTINATION "${sysroot_dir}"
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING
        PATTERN "*.so*"
        PATTERN "*.py*" EXCLUDE
        REGEX "gconv" EXCLUDE
        REGEX "audit" EXCLUDE
    )

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
endif()

# Install Sysroot (Skipping Empty Directories)
file(GLOB_RECURSE files LIST_DIRECTORIES FALSE RELATIVE "${sysroot_dir}" "${sysroot_dir}/*")
foreach(file IN LISTS files)
    get_filename_component(parent "${file}" DIRECTORY)
    install(PROGRAMS "${sysroot_dir}/${file}" DESTINATION "${MCPI_INSTALL_DIR}/sysroot/${parent}")
endforeach()
