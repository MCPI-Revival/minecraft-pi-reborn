project(media-layer-gles)

# Build
if(MCPI_USE_GLES1_COMPATIBILITY_LAYER)
    # GLESv1_CM Compatibility Layer
    add_library(GLESv1_CM INTERFACE)
    target_link_libraries(GLESv1_CM INTERFACE gles-compatibility-layer)
else()
    # Passthrough To glfwGetProcAddress()
    add_library(GLESv1_CM OBJECT src/passthrough.c)
    target_link_libraries(GLESv1_CM PRIVATE glfw media-layer-headers)
endif()
