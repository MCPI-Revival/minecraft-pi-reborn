project(libpng)

# Silence Warnings
add_compile_options(-w)

## LibPNG

# Options
set(PNG_TESTS FALSE CACHE BOOL "" FORCE)
set(PNG_STATIC ${BUILD_NATIVE_COMPONENTS} CACHE BOOL "" FORCE)
set(PNG_SHARED ${BUILD_ARM_COMPONENTS} CACHE BOOL "" FORCE)
if(BUILD_NATIVE_COMPONENTS)
    set(PNG_NO_STDIO FALSE CACHE BOOL "" FORCE)
else()
    set(PNG_NO_STDIO TRUE CACHE BOOL "" FORCE)
endif()

# Download
set(ZLIB_LIBRARY zlibstatic)
set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../zlib/src" "${CMAKE_CURRENT_BINARY_DIR}/../zlib/src")
set(CMAKE_POLICY_DEFAULT_CMP0054 OLD) # Silence Warning
add_subdirectory(src EXCLUDE_FROM_ALL)
set(CMAKE_POLICY_DEFAULT_CMP0054 NEW) # Re-Enable New Behavior
if(TARGET png12)
    set_target_properties(png12 PROPERTIES LINK_OPTIONS "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libpng.vers") # Use Symbol Versioning
    set_target_properties(png12 PROPERTIES DEBUG_POSTFIX "") # Fix LibPNG Suffix In Debug Mode
endif()
if(TARGET png12_static)
    find_library(M_LIBRARY NAMES m REQUIRED)
    target_link_libraries(png12_static "${ZLIB_LIBRARY}" "${M_LIBRARY}")
    target_include_directories(png12_static PUBLIC src)
endif()

# Ensure Build
if(TARGET png12)
    add_custom_target(png12-build ALL DEPENDS png12)
endif()
if(TARGET png12_static)
    add_custom_target(png12_static-build ALL DEPENDS png12_static)
endif()
# Install
if(TARGET png12)
    install(TARGETS png12 DESTINATION "${MCPI_LIB_DIR}")
endif()
