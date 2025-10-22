project(imgui-glad)

# Directories
set_and_mkdir(GLAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/glad")
set_and_mkdir(GLAD_SRC_DIR "${GLAD_DIR}/src")
set_and_mkdir(GLAD_INCLUDE_DIR "${GLAD_DIR}/include")

# Files
set(GLAD_SOURCES
    "${GLAD_INCLUDE_DIR}/KHR/khrplatform.h"
    "${GLAD_INCLUDE_DIR}/glad/glad.h"
    "${GLAD_SRC_DIR}/glad.c"
)

# Find Python
find_package(Python REQUIRED QUIET)

# Generate
add_custom_command(OUTPUT ${GLAD_SOURCES}
    COMMAND "${Python_EXECUTABLE}"
    ARGS "-m" "glad"
        "--out" "${GLAD_DIR}"
        "--api" "gl=1.1"
        "--generator" "c"
        "--reproducible"
        "--quiet"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
    VERBATIM
)

# Build
add_library(glad SHARED ${GLAD_SOURCES})
target_compile_definitions(glad
    PUBLIC GLAD_GLAPI_EXPORT
    PRIVATE GLAD_GLAPI_EXPORT_BUILD
)

# Link
if(NOT MCPI_WIN32)
    target_link_libraries(glad PRIVATE dl)
endif()

# Headers
setup_header_dirs(glad
    "${GLAD_INCLUDE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Install
setup_library(glad TRUE FALSE)