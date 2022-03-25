project(media-layer-stubs)

# Add GLES1 Stubs For Linking
add_library(GLESv1_CM SHARED src/GLESv1_CM.c)
target_link_libraries(GLESv1_CM media-layer-headers)
set_target_properties(GLESv1_CM PROPERTIES SOVERSION "1")

# Stubs Only Needed For ARM
if(BUILD_ARM_COMPONENTS)
    # Install Fake GLESv1_CM Stubs In Server Mode
    if(MCPI_HEADLESS_MODE)
        install(TARGETS GLESv1_CM DESTINATION "${MCPI_LIB_DIR}")
    endif()
endif()
