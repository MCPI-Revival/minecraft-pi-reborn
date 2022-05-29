project(media-layer-stubs)

# Stubs Only Needed For ARM
if(MCPI_USE_GLES1_COMPATIBILITY_LAYER AND BUILD_NATIVE_COMPONENTS AND NOT MCPI_HEADLESS_MODE)
    # GLESv1_CM Compatibility Layer
    set(GLES1_LINK_MODE "SHARED")
    if(MCPI_USE_MEDIA_LAYER_PROXY)
        # Link To Media Layer Proxy Client Statically
        # (This is so it doesn't interfere with the Media Layer Proxy Server's libGLESv1_CM.so.1 symlink.)
        set(GLES1_LINK_MODE "OBJECT")
    endif()
    add_library(GLESv1_CM "${GLES1_LINK_MODE}" src/compatibility-layer/state.c src/compatibility-layer/passthrough.c src/compatibility-layer/matrix.c src/compatibility-layer/draw.c)
    target_link_libraries(GLESv1_CM glfw reborn-util dl m)
    # Install
    if(NOT MCPI_USE_MEDIA_LAYER_PROXY)
        install(TARGETS GLESv1_CM DESTINATION "${MCPI_LIB_DIR}")
    endif()
else()
    # Add GLESv1_CM Stubs For Linking
    add_library(GLESv1_CM SHARED src/stubs.c)
    # Install Fake GLESv1_CM Stubs In Server Mode
    if(MCPI_HEADLESS_MODE AND BUILD_ARM_COMPONENTS)
        install(TARGETS GLESv1_CM DESTINATION "${MCPI_LIB_DIR}")
    endif()
endif()

# Common
target_link_libraries(GLESv1_CM media-layer-headers)
set_target_properties(GLESv1_CM PROPERTIES SOVERSION "1")
