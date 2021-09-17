# Compile For ARM64
if(NOT (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64_be" OR CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64" OR CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "armv8b" OR CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "armv8l"))
    # Use ARM64 Cross-Compiler
    include("${CMAKE_CURRENT_LIST_DIR}/base-toolchain.cmake")
    setup_toolchain("aarch64-linux-gnu")
endif()
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "aarch64")
