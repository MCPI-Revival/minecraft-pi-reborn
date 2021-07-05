project(media-layer-stubs)

# Add GLES Stubs For Linking
add_library(GLESv1_CM SHARED src/GLESv1_CM.c)
target_link_libraries(GLESv1_CM media-layer-headers)
set_target_properties(GLESv1_CM PROPERTIES SOVERSION "1")

if(BUILD_ARM_COMPONENTS)
    # Stub RPI-Specific Graphics
    add_library(bcm_host SHARED src/bcm_host.c)
    # Install
    install(TARGETS bcm_host DESTINATION "${MCPI_LIB_DIR}")

    # Stub EGL
    add_library(EGL SHARED src/EGL.c)
    target_link_libraries(EGL reborn-headers media-layer-headers)
    # Stub X11
    add_library(X11 SHARED src/X11.c)
    target_link_libraries(X11 reborn-headers media-layer-headers)
    set_target_properties(X11 PROPERTIES SOVERSION "6")
    # Install
    if(MCPI_SERVER_MODE OR MCPI_USE_MEDIA_LAYER_PROXY)
        install(TARGETS EGL X11 DESTINATION "${MCPI_LIB_DIR}")
    else()
        install(TARGETS EGL X11 DESTINATION "${MCPI_FALLBACK_LIB_DIR}") # Place At The End Of LD_LIBRARY_PATH
    endif()

    # Install GLESv1_CM Stubs In Server Mode
    if(MCPI_SERVER_MODE)
        install(TARGETS GLESv1_CM DESTINATION "${MCPI_LIB_DIR}")
    endif()

    # Add NOP GLESv2 That Dpends On Actual GLESv1_CM (This Cannot Be A Symlink Because The Location Of GLESv1_CM Is Dynamic)
    add_library(GLESv2 SHARED src/nop.c)
    target_compile_options(GLESv2 PRIVATE -w)
    target_link_libraries(GLESv2 GLESv1_CM)
    # Force Link
    target_link_options(GLESv2 PRIVATE "-Wl,--no-as-needed")
    # Install
    install(TARGETS GLESv2 DESTINATION "${MCPI_LIB_DIR}")
endif()