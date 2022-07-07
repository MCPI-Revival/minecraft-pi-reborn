# Compile For x86_64
include("${CMAKE_CURRENT_LIST_DIR}/base-toolchain.cmake")
# Use x86_64 Cross-Compiler
setup_toolchain("x86_64-linux-gnu")
# Details
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "x86_64")
