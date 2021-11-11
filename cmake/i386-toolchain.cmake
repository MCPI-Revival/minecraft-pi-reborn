# Compile For i386
include("${CMAKE_CURRENT_LIST_DIR}/base-toolchain.cmake")
# Use i386 Cross-Compiler
setup_toolchain("i386-linux-gnu")
# Details
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "i386")
