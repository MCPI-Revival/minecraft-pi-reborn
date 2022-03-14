project(launcher)

# Launcher
if(BUILD_NATIVE_COMPONENTS)
    add_executable(launcher src/bootstrap.c src/ldconfig.cpp)
    if(MCPI_SERVER_MODE)
        target_sources(launcher PRIVATE src/server/launcher.c)
    else()
        target_sources(launcher PRIVATE src/client/launcher.cpp)
    endif()
    target_link_libraries(launcher reborn-util)
    # Install
    install(TARGETS launcher DESTINATION "${MCPI_INSTALL_DIR}")
    install_symlink("../${MCPI_INSTALL_DIR}/launcher" "bin/${MCPI_VARIANT_NAME}")
    if(NOT MCPI_SERVER_MODE)
        install(DIRECTORY "client-data/" DESTINATION ".")
    else()
        set(ICON_PATH "share/icons/hicolor/scalable/apps")
        install(
            FILES "client-data/${ICON_PATH}/com.thebrokenrail.MCPIReborn.png"
            DESTINATION "${ICON_PATH}"
            RENAME "com.thebrokenrail.MCPIRebornServer.png"
        )
    endif()
endif()
