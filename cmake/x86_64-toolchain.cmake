# Compile For x86_64
if(NOT CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
    # Use x86_64 Cross-Compiler
    include("${CMAKE_CURRENT_LIST_DIR}/base-toolchain.cmake")
    setup_toolchain("x86_64-linux-gnu")
endif()
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "x86_64")
