project(imgui-glad)

# Setup GLAD
set(MESSAGE_QUIET TRUE)
add_subdirectory(src/cmake)
glad_add_library(glad SHARED REPRODUCIBLE QUIET API gl:compatibility=1.1)
set(MESSAGE_QUIET FALSE)

# Fix GL/gl.h
setup_header_dirs(glad "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Install
setup_library(glad TRUE FALSE)