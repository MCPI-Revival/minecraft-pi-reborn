project(media-layer-gles)

# Dependencies
add_subdirectory(dependencies)

# Build
if(MCPI_HEADLESS_MODE)
    # Stubs For Headless Mode
    add_library(GLESv1_CM OBJECT src/stubs.c)
    target_link_libraries(GLESv1_CM PRIVATE media-layer-headers)
elseif(MCPI_USE_GLES1_COMPATIBILITY_LAYER)
    # GLESv1_CM Compatibility Layer
    add_library(GLESv1_CM INTERFACE)
    target_link_libraries(GLESv1_CM INTERFACE gles-compatibility-layer)
else()
    # Passthrough To glfwGetProcAddress()
    add_library(GLESv1_CM OBJECT src/passthrough.c)
    target_link_libraries(GLESv1_CM PRIVATE glfw media-layer-headers)
endif()

# SDK
if(BUILD_ARM_COMPONENTS)
    install(TARGETS GLESv1_CM EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
    if(MCPI_USE_GLES1_COMPATIBILITY_LAYER)
        install(TARGETS gles-compatibility-layer EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
    endif()
endif()
