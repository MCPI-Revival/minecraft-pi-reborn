# Target
set(target "arm-linux-gnueabihf")

# Pick Archive
get_arch(arch)
set(toolchain_name "arm-toolchain-${arch}")
set(toolchain_dir "${CMAKE_CURRENT_BINARY_DIR}/${toolchain_name}")
set(toolchain_file "${CMAKE_CURRENT_LIST_DIR}/../../../archives/${toolchain_name}.tar.xz")

# Sysroot
include("${CMAKE_CURRENT_LIST_DIR}/sysroot.cmake")

# Extract If Needed
file(SHA256 "${toolchain_file}" toolchain_hash)
if(NOT (DEFINED CACHE{MCPI_TOOLCHAIN_HASH} AND toolchain_hash STREQUAL MCPI_TOOLCHAIN_HASH))
    force_set(MCPI_TOOLCHAIN_HASH "${toolchain_hash}" INTERNAL)
    message(STATUS "Extracting Toolchain...")
    file(REMOVE_RECURSE "${toolchain_dir}")
    file(ARCHIVE_EXTRACT INPUT "${toolchain_file}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}" TOUCH)

    # Force Toolchain
    file(WRITE "${toolchain_dir}/toolchain.cmake"
        "set(CMAKE_C_COMPILER \"\${CMAKE_CURRENT_LIST_DIR}/bin/${target}-gcc\")\n"
        "set(CMAKE_CXX_COMPILER \"\${CMAKE_CURRENT_LIST_DIR}/bin/${target}-g++\")\n"
        "set(CMAKE_SYSTEM_NAME \"Linux\")\n"
        "set(CMAKE_SYSTEM_PROCESSOR \"arm\")\n"
        "set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)\n"
    )
    force_set(MCPI_CMAKE_TOOLCHAIN_FILE "${toolchain_dir}/toolchain.cmake" FILEPATH)
    set(MCPI_FORCE_ARM_REBUILD TRUE)

    # Sysroot
    build_sysroot()
endif()