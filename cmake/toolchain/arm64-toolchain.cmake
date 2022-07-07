# Compile For ARM64
include("${CMAKE_CURRENT_LIST_DIR}/base-toolchain.cmake")
# Use ARM64 Cross-Compiler
setup_toolchain("aarch64-linux-gnu")
# Details
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "aarch64")
