# Compile For x86_64
include("${CMAKE_CURRENT_LIST_DIR}/base-toolchain.cmake")
if(NOT HOST_ARCHITECTURE STREQUAL "x86_64")
    # Use x86_64 Cross-Compiler
    setup_toolchain("x86_64-linux-gnu")
endif()
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "x86_64")
