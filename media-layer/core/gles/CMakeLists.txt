project(media-layer-gles)

# Build
if(MCPI_HEADLESS_MODE)
    # Stubs For Headless Mode
    set(GLES_SRC src/stubs.c)
elseif(MCPI_USE_GLES1_COMPATIBILITY_LAYER)
    # GLESv1_CM Compatibility Layer
    set(GLES_SRC src/compatibility-layer/state.c src/compatibility-layer/passthrough.c src/compatibility-layer/matrix.c src/compatibility-layer/draw.c)
else()
    # Passthrough To glfwGetProcAddress()
    set(GLES_SRC src/passthrough.c)
endif()
add_library(GLESv1_CM OBJECT ${GLES_SRC})
if(NOT MCPI_HEADLESS_MODE)
    target_link_libraries(GLESv1_CM PRIVATE glfw PUBLIC reborn-util PRIVATE dl PRIVATE m)
    # Shaders
    if(MCPI_USE_GLES1_COMPATIBILITY_LAYER)
        embed_resource(GLESv1_CM src/compatibility-layer/shaders/main.vsh)
        embed_resource(GLESv1_CM src/compatibility-layer/shaders/main.fsh)
    endif()
endif()

# Common
target_link_libraries(GLESv1_CM PUBLIC media-layer-headers)
set_target_properties(GLESv1_CM PROPERTIES SOVERSION "1")
# Install
install(TARGETS GLESv1_CM DESTINATION "${MCPI_LIB_DIR}")
# SDK
if(BUILD_ARM_COMPONENTS)
    install(TARGETS GLESv1_CM EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
endif()
