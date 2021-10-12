project(zlib)

include(FetchContent)

# Silence Warnings
add_compile_options(-w)

## zlib

# Download
set(SKIP_INSTALL_ALL TRUE) # Skip Default ZLib Installation
FetchContent_Declare(
    zlib
    GIT_REPOSITORY "https://github.com/madler/zlib.git"
    GIT_TAG "v1.2.11"
)
FetchContent_Populate(zlib)
include_directories("${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}") # Fix ZLib Build
add_subdirectory("${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}" EXCLUDE_FROM_ALL)

# Install
install(TARGETS zlib DESTINATION "${MCPI_LIB_DIR}")

