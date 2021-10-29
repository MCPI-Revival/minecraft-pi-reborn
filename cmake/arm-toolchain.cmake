# Compile For ARM
include("${CMAKE_CURRENT_LIST_DIR}/base-toolchain.cmake")
if(HOST_ARCHITECTURE STREQUAL "aarch64_be" OR HOST_ARCHITECTURE STREQUAL "aarch64" OR HOST_ARCHITECTURE STREQUAL "armv8b" OR HOST_ARCHITECTURE STREQUAL "armv8l")
    # Force 32-Bit Compile
    add_compile_options("-m32")
elseif((NOT HOST_ARCHITECTURE STREQUAL "arm") AND (NOT HOST_ARCHITECTURE STREQUAL "armv7l"))
    # Use ARM Cross-Compiler
    setup_toolchain("arm-linux-gnueabihf")
endif()
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "arm")
