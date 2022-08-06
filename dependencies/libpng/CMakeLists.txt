project(libpng)

# ZLib (Needed By libpng)
add_subdirectory(zlib)

# Silence Warnings
add_compile_options(-w)

## LibPNG

# Options
set(PNG_TESTS FALSE CACHE BOOL "" FORCE)
set(PNG_NO_STDIO FALSE CACHE BOOL "" FORCE)
set(PNG_BUILD_ZLIB TRUE CACHE BOOL "" FORCE)

# Download
set(ZLIB_LIBRARY zlibstatic)
set(ZLIB_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/zlib/src" "${CMAKE_CURRENT_BINARY_DIR}/zlib/src")
add_subdirectory(src EXCLUDE_FROM_ALL)

# Use Symbol Versioning
set_target_properties(png12 PROPERTIES LINK_OPTIONS "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libpng.vers")

# Ensure Build
add_custom_target(png12-build ALL DEPENDS png12)
# Install
install(TARGETS png12 DESTINATION "${MCPI_LIB_DIR}")
if(BUILD_ARM_COMPONENTS)
    install(TARGETS png12 zlibstatic EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
endif()

# License
install(FILES src/LICENSE DESTINATION "${MCPI_LEGAL_DIR}/libpng")
