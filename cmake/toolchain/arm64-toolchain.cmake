# Use ARM64 Cross-Compiler
set(target "aarch64-linux-gnu")
include("${CMAKE_CURRENT_LIST_DIR}/base-toolchain.cmake")

# Details
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "aarch64")