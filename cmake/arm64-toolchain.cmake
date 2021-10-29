# Compile For ARM64
include("${CMAKE_CURRENT_LIST_DIR}/base-toolchain.cmake")
if(NOT (HOST_ARCHITECTURE STREQUAL "aarch64_be" OR HOST_ARCHITECTURE STREQUAL "aarch64" OR HOST_ARCHITECTURE STREQUAL "armv8b" OR HOST_ARCHITECTURE STREQUAL "armv8l"))
    # Use ARM64 Cross-Compiler
    setup_toolchain("aarch64-linux-gnu")
endif()
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "aarch64")
