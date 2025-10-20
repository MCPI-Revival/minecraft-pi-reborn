project(glfw)

# Silence Warnings
add_compile_options(-w)

## GLFW

# Download
force_set(GLFW_BUILD_EXAMPLES FALSE BOOL)
force_set(GLFW_BUILD_TESTS FALSE BOOL)
force_set(GLFW_BUILD_DOCS FALSE BOOL)
force_set(GLFW_INSTALL FALSE BOOL)
if(MCPI_WIN32)
    force_set(GLFW_BUILD_WIN32 TRUE BOOL)
    force_set(GLFW_BUILD_X11 FALSE BOOL)
    force_set(GLFW_BUILD_WAYLAND FALSE BOOL)
else()
    force_set(GLFW_BUILD_WIN32 FALSE BOOL)
    force_set(GLFW_BUILD_X11 TRUE BOOL)
    force_set(GLFW_BUILD_WAYLAND TRUE BOOL)
endif()
force_set(GLFW_BUILD_COCOA FALSE BOOL)
force_set(GLFW_LIBRARY_TYPE "SHARED" STRING)
set(MESSAGE_QUIET TRUE)
add_subdirectory(src EXCLUDE_FROM_ALL SYSTEM)
unset(MESSAGE_QUIET)

# Install
setup_library(glfw TRUE TRUE)

# License
install(FILES src/LICENSE.md DESTINATION "${MCPI_LEGAL_DIR}/GLFW")
