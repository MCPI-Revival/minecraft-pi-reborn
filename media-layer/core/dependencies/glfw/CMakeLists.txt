project(glfw)

# Silence Warnings
add_compile_options(-w)

## GLFW

# Download
set(BUILD_SHARED_LIBS TRUE CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES FALSE CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS FALSE CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS FALSE CACHE BOOL "" FORCE)
set(GLFW_INSTALL FALSE CACHE BOOL "" FORCE)
set(GLFW_BUILD_WIN32 FALSE CACHE BOOL "" FORCE)
set(GLFW_BUILD_COCOA FALSE CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 TRUE CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND TRUE CACHE BOOL "" FORCE)
set(GLFW_LIBRARY_TYPE "SHARED" CACHE BOOL "" FORCE)
add_subdirectory(src EXCLUDE_FROM_ALL)

# Ensure Build
add_custom_target(glfw-build ALL DEPENDS glfw)

# Remove When glfw/glfw#2192 Is Merged
target_compile_definitions(glfw PRIVATE -D_GLFW_LINUX_JOYSTICK)
# Remove When glfw/glfw#2198 Is Merged
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/publicize-glfw-symbol.h"
    "#pragma once\n"
    "extern __attribute__((visibility(\"default\"))) void _glfwDetectJoystickConnectionLinux(void);\n"
)
target_compile_options(glfw PRIVATE -include PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/publicize-glfw-symbol.h")

# Install
install(TARGETS glfw DESTINATION "${MCPI_LIB_DIR}")
if(BUILD_ARM_COMPONENTS)
    install(TARGETS glfw EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
endif()

# License
install(FILES src/LICENSE.md DESTINATION "${MCPI_LEGAL_DIR}/glfw")
