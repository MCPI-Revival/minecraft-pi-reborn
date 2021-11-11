# Compile For ARM
include("${CMAKE_CURRENT_LIST_DIR}/base-toolchain.cmake")
# Use ARM Cross-Compiler
setup_toolchain("arm-linux-gnueabihf")
# Details
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "arm")
