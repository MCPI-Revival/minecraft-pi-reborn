# Warning
message(WARNING "i686 Builds Are Unsupported, Proceed At Your Own Risk")
# Compile For i686
include("${CMAKE_CURRENT_LIST_DIR}/base-toolchain.cmake")
# Use i686 Cross-Compiler
setup_toolchain("i686-linux-gnu")
# Details
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "i686")
