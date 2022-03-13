project(media-layer-stubs-redirect)

# MCPI Depends On GLESv2, But Uses GLESv1_CM
add_library(GLESv2_redirect SHARED src/nop.c)
target_link_libraries(GLESv2_redirect GLESv1_CM)
target_link_options(GLESv2_redirect PRIVATE "-Wl,--no-as-needed")
set_target_properties(GLESv2_redirect PROPERTIES OUTPUT_NAME "GLESv2")
install(TARGETS GLESv2_redirect DESTINATION "${MCPI_LIB_DIR}")

# MCPI links to libEGL.so instead of libEGL.so.1, this creates a fake libEGL.so which redirects it to the correct libEGL.so.1.
# This is only needed on configurations that don't generate their own libEGL.so.
if(NOT MCPI_HEADLESS_MODE AND NOT MCPI_USE_MEDIA_LAYER_PROXY)
    add_library(EGL_redirect SHARED src/nop.c)
    target_link_libraries(EGL_redirect EGL)
    target_link_options(EGL_redirect PRIVATE "-Wl,--no-as-needed")
    set_target_properties(EGL_redirect PROPERTIES OUTPUT_NAME "EGL")
    install(TARGETS EGL_redirect DESTINATION "${MCPI_LIB_DIR}")
endif()
