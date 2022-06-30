project(media-layer-stubs)

# Stubs Only Needed For ARM
if(BUILD_NATIVE_COMPONENTS AND NOT MCPI_HEADLESS_MODE)
    # GLESv1_CM Compatibility Layer
    set(GLES1_LINK_MODE "SHARED")
    if(MCPI_USE_MEDIA_LAYER_PROXY)
        # Link To Media Layer Proxy Client Statically
        # (This is so it doesn't interfere with the Media Layer Proxy Server's libGLESv1_CM.so.1 symlink.)
        set(GLES1_LINK_MODE "OBJECT")
    endif()
    if(MCPI_USE_GLES1_COMPATIBILITY_LAYER)
        set(GLES_SRC src/compatibility-layer/state.c src/compatibility-layer/passthrough.c src/compatibility-layer/matrix.c src/compatibility-layer/draw.c src/compatibility-layer/buffer.cpp)
    else()
        set(GLES_SRC src/passthrough.c)
    endif()
    add_library(GLESv1_CM "${GLES1_LINK_MODE}" ${GLES_SRC})
    target_link_libraries(GLESv1_CM PRIVATE glfw PUBLIC reborn-util PRIVATE dl PRIVATE m)
    # Install
    if(GLES1_LINK_MODE STREQUAL "SHARED")
        install(TARGETS GLESv1_CM DESTINATION "${MCPI_LIB_DIR}")
    endif()
else()
    # Add GLESv1_CM Stubs For Linking
    add_library(GLESv1_CM SHARED src/stubs.c)
    # Install Fake GLESv1_CM Stubs In Server Mode
    if(BUILD_ARM_COMPONENTS AND MCPI_HEADLESS_MODE)
        install(TARGETS GLESv1_CM DESTINATION "${MCPI_LIB_DIR}")
    endif()
endif()

# SDK
if(BUILD_ARM_COMPONENTS)
    install(TARGETS GLESv1_CM EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
endif()

# Common
target_link_libraries(GLESv1_CM PUBLIC media-layer-headers)
set_target_properties(GLESv1_CM PROPERTIES SOVERSION "1")
