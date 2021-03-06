project(libpng)

include(FetchContent)

# Silence Warnings
add_compile_options(-w)

## LibPNG

# Download
set(SKIP_INSTALL_ALL TRUE) # Skip Default LibPNG Installation
FetchContent_Declare(
    libpng
    GIT_REPOSITORY "https://git.code.sf.net/p/libpng/code"
    GIT_TAG "v1.2.59"
)
FetchContent_Populate(libpng)
set(ZLIB_LIBRARY zlib)
set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}")
set(CMAKE_POLICY_DEFAULT_CMP0054 OLD) # Silence Warning
add_subdirectory("${libpng_SOURCE_DIR}" "${libpng_BINARY_DIR}")
set(CMAKE_POLICY_DEFAULT_CMP0054 NEW) # Re-Enable New Behavior
set_target_properties(png12 PROPERTIES LINK_FLAGS "-Wl,--version-script='${CMAKE_CURRENT_SOURCE_DIR}/libpng.vers'") # Use Symbol Versioning
set_target_properties(png12 PROPERTIES DEBUG_POSTFIX "") # Fix LibPNG Suffix In Debug Mode

# Install
install(TARGETS png12 DESTINATION "${MCPI_LIB_DIR}")

