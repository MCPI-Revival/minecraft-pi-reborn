# Target
set(target "arm-linux-gnueabihf")

# Pick Archive
get_arch(arch)
set(toolchain_name "arm-toolchain-${arch}")
set(toolchain_file "${CMAKE_CURRENT_LIST_DIR}/../../../archives/${toolchain_name}.tar.xz")
file(SHA256 "${toolchain_file}" toolchain_hash)
set(toolchain_dir "${CMAKE_CURRENT_BINARY_DIR}/${toolchain_name}@${toolchain_hash}")

# Extract If Needed
force_set(MCPI_CMAKE_TOOLCHAIN_FILE "${toolchain_dir}/toolchain.cmake" FILEPATH)
if(NOT EXISTS "${MCPI_CMAKE_TOOLCHAIN_FILE}")
    message(STATUS "Extracting Toolchain...")
    file(ARCHIVE_EXTRACT
        INPUT "${toolchain_file}"
        DESTINATION "${toolchain_dir}"
        TOUCH
    )

    # Force Toolchain
    set(toolchain_bin "\${CMAKE_CURRENT_LIST_DIR}/${toolchain_name}/bin")
    file(WRITE "${MCPI_CMAKE_TOOLCHAIN_FILE}"
        "set(CMAKE_C_COMPILER \"${toolchain_bin}/${target}-gcc\")\n"
        "set(CMAKE_CXX_COMPILER \"${toolchain_bin}/${target}-g++\")\n"
        "set(CMAKE_SYSTEM_NAME \"Linux\")\n"
        "set(CMAKE_SYSTEM_PROCESSOR \"arm\")\n"
        "set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)\n"
    )
    set(MCPI_FORCE_ARM_REBUILD TRUE)
endif()

# Build Sysroot
include("${CMAKE_CURRENT_LIST_DIR}/sysroot.cmake")
build_sysroot()