# CPack
set(CPACK_PACKAGE_NAME "${MCPI_APP_NAME}")
set(CPACK_PACKAGE_VENDOR "${MCPI_AUTHOR} & Mojang AB")
set(CPACK_VERBATIM_VARIABLES TRUE)
set(CPACK_MONOLITHIC_INSTALL TRUE)
get_package_file_name(CPACK_PACKAGE_FILE_NAME "${MCPI_VERSION}")
get_package_file_name(CPACK_PACKAGE_FILE_NAME_ZSYNC "latest")

# Version
string(REPLACE "." ";" VERSION_LIST "${MCPI_VERSION}")
list(GET VERSION_LIST 0 CPACK_PACKAGE_VERSION_MAJOR)
list(GET VERSION_LIST 1 CPACK_PACKAGE_VERSION_MINOR)
list(GET VERSION_LIST 2 CPACK_PACKAGE_VERSION_PATCH)

# Configure Generator
if(MCPI_IS_APPIMAGE_BUILD)
    # AppImage
    set(CPACK_GENERATOR "External")
    set(CPACK_EXTERNAL_ENABLE_STAGING TRUE)
    set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/appimage.cmake")
    # Pass Variable To CPack
    macro(pass_to_cpack var)
        set("CPACK_MCPI_${var}" "${MCPI_${var}}")
    endmacro()
    pass_to_cpack(VERSION)
    pass_to_cpack(ARCH)
    pass_to_cpack(REPO)
    pass_to_cpack(APPIMAGE_EXT)
    pass_to_cpack(APPIMAGE_ZSYNC_EXT)
elseif(MCPI_IS_DEBIAN_BUILD)
    # Debian
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${MCPI_ARCH}")
    set(CPACK_DEBIAN_PACKAGE_SECTION "games")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libstdc++6, libopenal1, libglib2.0-0t64 | libglib2.0-0")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${MCPI_AUTHOR}")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${MCPI_REPO}")
endif()

# Package
include(CPack)
