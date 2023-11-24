project(images)

# Title Background
if(NOT MCPI_HEADLESS_MODE)
    install(
        FILES "background.png"
        DESTINATION "${MCPI_INSTALL_DIR}/data/images/gui"
        RENAME "titleBG.png"
    )
endif()

# Chest Model
if(NOT MCPI_HEADLESS_MODE)
    install(
        FILES "chest.png"
        DESTINATION "${MCPI_INSTALL_DIR}/data/images/item"
    )
endif()

# Icon
install(
    FILES "icon.png"
    DESTINATION "${MCPI_SHARE_DIR}/icons/hicolor/scalable/apps"
    RENAME "${MCPI_APP_ID}.png"
)

# AppImage
if(MCPI_IS_APPIMAGE_BUILD)
    install_symlink("${MCPI_SHARE_DIR}/icons/hicolor/scalable/apps/${MCPI_APP_ID}.png" "${MCPI_APP_ID}.png")
    install_symlink("${MCPI_APP_ID}.png" ".DirIcon")
endif()
