project(images)

# Title Background
if(NOT MCPI_SERVER_MODE)
    install(
        FILES "background.png"
        DESTINATION "${MCPI_INSTALL_DIR}/data/images/gui"
        RENAME "titleBG.png"
    )
endif()

# Icon
install(
    FILES "icon.png"
    DESTINATION "share/icons/hicolor/scalable/apps"
    RENAME "${MCPI_APP_ID}.png"
)
