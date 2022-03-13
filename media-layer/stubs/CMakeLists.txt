project(media-layer-stubs)

# Add GLES1 Stubs For Linking
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

    # Install Fake GLESv1_CM Stubs In Server Mode
    if(MCPI_HEADLESS_MODE)
        install(TARGETS GLESv1_CM DESTINATION "${MCPI_LIB_DIR}")
    endif()

    # Redirect MCPI To Correct Libraries
    add_subdirectory(redirect)
endif()
