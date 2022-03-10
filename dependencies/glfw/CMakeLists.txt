project(glfw)

# Silence Warnings
add_compile_options(-w)

## GLFW

# Download
set(BUILD_SHARED_LIBS FALSE)
set(GLFW_BUILD_EXAMPLES FALSE)
set(GLFW_BUILD_TESTS FALSE)
set(GLFW_BUILD_DOCS FALSE)
set(GLFW_INSTALL FALSE)
set(GLFW_BUILD_WIN32 FALSE)
set(GLFW_BUILD_COCOA FALSE)
set(GLFW_BUILD_X11 TRUE)
set(GLFW_BUILD_WAYLAND TRUE)
set(GLFW_LIBRARY_TYPE "STATIC")
add_subdirectory(src EXCLUDE_FROM_ALL)

# Ensure Build
add_custom_target(glfw-build ALL DEPENDS glfw)
