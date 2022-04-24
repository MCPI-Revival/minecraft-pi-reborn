project(libpng)

# Silence Warnings
add_compile_options(-w)

## LibPNG

# Download
set(ZLIB_LIBRARY zlibstatic)
set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/src" "${CMAKE_CURRENT_BINARY_DIR}/../zlib/src")
set(CMAKE_POLICY_DEFAULT_CMP0054 OLD) # Silence Warning
add_subdirectory(src EXCLUDE_FROM_ALL)
set(CMAKE_POLICY_DEFAULT_CMP0054 NEW) # Re-Enable New Behavior
set_target_properties(png12 PROPERTIES LINK_OPTIONS "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libpng.vers") # Use Symbol Versioning
set_target_properties(png12 PROPERTIES DEBUG_POSTFIX "") # Fix LibPNG Suffix In Debug Mode

# Ensure Build
add_custom_target(png12-build ALL DEPENDS png12)
# Install
install(TARGETS png12 DESTINATION "${MCPI_LIB_DIR}")
