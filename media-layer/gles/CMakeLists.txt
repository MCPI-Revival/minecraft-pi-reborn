project(media-layer-stubs)

# Stubs Only Needed For ARM
if(NOT MCPI_HEADLESS_MODE AND ((BUILD_NATIVE_COMPONENTS AND MCPI_USE_MEDIA_LAYER_PROXY) OR (BUILD_ARM_COMPONENTS AND NOT MCPI_USE_MEDIA_LAYER_PROXY)))
    # GLESv1_CM Compatibility Layer
    if(MCPI_USE_GLES1_COMPATIBILITY_LAYER)
        set(GLES_SRC src/compatibility-layer/state.c src/compatibility-layer/passthrough.c src/compatibility-layer/matrix.c src/compatibility-layer/draw.c src/compatibility-layer/buffer.cpp)
    else()
        set(GLES_SRC src/passthrough.c)
    endif()
    add_library(GLESv1_CM SHARED ${GLES_SRC})
    target_link_libraries(GLESv1_CM PRIVATE glfw PUBLIC reborn-util PRIVATE dl PRIVATE m)
    # Install
    install(TARGETS GLESv1_CM DESTINATION "${MCPI_LIB_DIR}")
elseif(BUILD_ARM_COMPONENTS)
    # Add GLESv1_CM Stubs For Linking
    add_library(GLESv1_CM SHARED src/stubs.c)
    # Install Fake GLESv1_CM Stubs In Server Mode
    if(MCPI_HEADLESS_MODE)
        install(TARGETS GLESv1_CM DESTINATION "${MCPI_LIB_DIR}")
    endif()
endif()

# Common
if(TARGET GLESv1_CM)
    target_link_libraries(GLESv1_CM PUBLIC media-layer-headers)
    set_target_properties(GLESv1_CM PROPERTIES SOVERSION "1")
    # SDK
    if(BUILD_ARM_COMPONENTS)
        install(TARGETS GLESv1_CM EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
    endif()
endif()
