project(launcher)

# Launcher
if(BUILD_NATIVE_COMPONENTS)
    add_executable(launcher src/bootstrap.c src/patchelf.c src/crash-report.c)
    if(MCPI_SERVER_MODE)
        target_sources(launcher PRIVATE src/server/launcher.c)
    else()
        target_sources(launcher PRIVATE src/client/launcher.cpp)
    endif()
    target_link_libraries(launcher reborn-util)
    # Install
    install(TARGETS launcher DESTINATION "${MCPI_INSTALL_DIR}")
    install_symlink("../${MCPI_INSTALL_DIR}/launcher" "bin/${MCPI_VARIANT_NAME}")
    set(ICON_PATH "data/com.thebrokenrail.MCPIReborn.png")
    set(ICON_TARGET_PATH "share/icons/hicolor/scalable/apps")
    if(NOT MCPI_SERVER_MODE)
        install(DIRECTORY "data/client/" DESTINATION ".")
        install(
            FILES "${ICON_PATH}"
            DESTINATION "${ICON_TARGET_PATH}"
            RENAME "com.thebrokenrail.MCPIRebornClient.png"
        )
    else()
        install(DIRECTORY "data/server/" DESTINATION ".")
        install(
            FILES "${ICON_PATH}"
            DESTINATION "${ICON_TARGET_PATH}"
            RENAME "com.thebrokenrail.MCPIRebornServer.png"
        )
    endif()
endif()
