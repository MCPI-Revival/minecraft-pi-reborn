project(libpng)

# ZLib (Needed By libpng)
add_subdirectory(zlib)

# Silence Warnings
add_compile_options(-w)

## LibPNG

# Options
set(PNG_TESTS FALSE CACHE BOOL "" FORCE)
set(PNG_NO_STDIO FALSE CACHE BOOL "" FORCE)
set(PNG_STATIC FALSE CACHE BOOL "" FORCE)
set(PNG_SHARED TRUE CACHE BOOL "" FORCE)

# Download
set(ZLIB_LIBRARY zlibstatic)
set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zlib/src")
set(CMAKE_POLICY_DEFAULT_CMP0054 OLD) # Silence Warning
set(CMAKE_POLICY_DEFAULT_CMP0003 NEW) # Silence Warning
set(CMAKE_POLICY_DEFAULT_CMP0022 NEW) # Fix Error
set(M_LIBRARY m) # No Full Paths!
add_subdirectory(src EXCLUDE_FROM_ALL)
set(CMAKE_POLICY_DEFAULT_CMP0054 NEW) # Re-Enable New Behavior

# Setup Target
set_target_properties(png12 PROPERTIES LINK_OPTIONS "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libpng.vers") # Use Symbol Versioning
set_target_properties(png12 PROPERTIES DEBUG_POSTFIX "") # Fix LibPNG Suffix In Debug Mode
target_include_directories(png12 PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>")
foreach(zlib_include_dir IN ITEMS "${ZLIB_INCLUDE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/zlib/src")
    target_include_directories(png12 PUBLIC "$<BUILD_INTERFACE:${zlib_include_dir}>")
endforeach()

# Ensure Build
add_custom_target(png12-build ALL DEPENDS png12)
# Install
install(TARGETS png12 DESTINATION "${MCPI_LIB_DIR}")
if(BUILD_ARM_COMPONENTS)
    install(TARGETS png12 zlibstatic EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
endif()

# License
install(FILES src/LICENSE DESTINATION "${MCPI_LEGAL_DIR}/libpng")
