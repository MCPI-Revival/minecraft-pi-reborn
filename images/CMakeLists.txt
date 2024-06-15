project(images)

# Title Background
install(
    FILES "background.png"
    DESTINATION "${MCPI_INSTALL_DIR}/data/images/gui"
    RENAME "titleBG.png"
)

# Chest Model
install(
    FILES "chest.png"
    DESTINATION "${MCPI_INSTALL_DIR}/data/images/item"
)

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
