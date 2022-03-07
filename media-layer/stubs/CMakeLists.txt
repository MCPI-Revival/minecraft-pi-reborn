project(media-layer-stubs)

# Add GLES Stubs For Linking
add_library(GLESv1_CM SHARED src/GLESv1_CM.c)
target_link_libraries(GLESv1_CM media-layer-headers)
set_target_properties(GLESv1_CM PROPERTIES SOVERSION "1")

# Stubs Only Needed For ARM
if(BUILD_ARM_COMPONENTS)
    # Stub RPI-Specific Graphics
    add_library(bcm_host SHARED src/bcm_host.c)
    # Install
    install(TARGETS bcm_host DESTINATION "${MCPI_LIB_DIR}")

    # Stub EGL
    add_library(EGL SHARED src/EGL.c)
    target_link_libraries(EGL reborn-headers media-layer-headers)
    set_target_properties(EGL PROPERTIES SOVERSION "1")
    # Stub X11
    add_library(X11 SHARED src/X11.c)
    target_link_libraries(X11 reborn-headers media-layer-headers)
    set_target_properties(X11 PROPERTIES SOVERSION "6")
    # Install
    if(MCPI_HEADLESS_MODE OR MCPI_USE_MEDIA_LAYER_PROXY)
        install(TARGETS EGL X11 DESTINATION "${MCPI_LIB_DIR}")
    else()
        install(TARGETS EGL X11 DESTINATION "${MCPI_FALLBACK_LIB_DIR}") # Place At The End Of LD_LIBRARY_PATH
    endif()

    # Install GLESv1_CM Stubs In Server Mode
    if(MCPI_HEADLESS_MODE)
        install(TARGETS GLESv1_CM DESTINATION "${MCPI_LIB_DIR}")
    endif()

    # MCPI Depends On GLESv2, But Uses GLESv1_CM
    install_symlink("libGLESv1_CM.so.1" "${MCPI_LIB_DIR}/libGLESv2.so")
    # Prevent MCPI From Linking To The Legacy GL Driver When Directly Linking To GL (RPI-Specific)
    if(NOT MCPI_HEADLESS_MODE AND NOT MCPI_USE_MEDIA_LAYER_PROXY)
        # Symlinks
        install_symlink("/usr/lib/arm-linux-gnueabihf/libEGL.so.1" "${MCPI_LIB_DIR}/libEGL.so")
        install_symlink("/usr/lib/arm-linux-gnueabihf/libGLESv1_CM.so.1" "${MCPI_LIB_DIR}/libGLESv1_CM.so.1")
    endif()
endif()
