# Use ARM Cross-Compiler
set(target "arm-linux-gnueabihf")
include("${CMAKE_CURRENT_LIST_DIR}/base-toolchain.cmake")

# Details
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "arm")