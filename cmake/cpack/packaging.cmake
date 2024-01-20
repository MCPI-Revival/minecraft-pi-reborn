# Determine Architecture
set(CPACK_MCPI_ARCH "unknown")
include(CheckSymbolExists)
function(check_arch symbol name)
    set(CMAKE_REQUIRED_QUIET TRUE)
    check_symbol_exists("${symbol}" "" "IS_ARCH_${name}")
    unset(CMAKE_REQUIRED_QUIET)
    if("${IS_ARCH_${name}}")
        set(CPACK_MCPI_ARCH "${name}" PARENT_SCOPE)
    endif()
endfunction()
check_arch("__arm__" "armhf")
check_arch("__aarch64__" "arm64")
check_arch("__x86_64__" "amd64")

# CPack
set(CPACK_PACKAGE_NAME "${MCPI_VARIANT_NAME}")
set(CPACK_PACKAGE_VENDOR "TheBrokenRail & Mojang AB")
set(CPACK_VERBATIM_VARIABLES TRUE)
set(CPACK_MONOLITHIC_INSTALL TRUE)
set(CPACK_PACKAGE_FILE_NAME "${MCPI_VARIANT_NAME}-${MCPI_VERSION}-${CPACK_MCPI_ARCH}")
set(CPACK_PACKAGE_FILE_NAME_ZSYNC "${MCPI_VARIANT_NAME}-latest-${CPACK_MCPI_ARCH}")

# Version
string(REPLACE "." ";" VERSION_LIST "${MCPI_VERSION}")
list(GET VERSION_LIST 0 CPACK_PACKAGE_VERSION_MAJOR)
list(GET VERSION_LIST 1 CPACK_PACKAGE_VERSION_MINOR)
list(GET VERSION_LIST 2 CPACK_PACKAGE_VERSION_PATCH)

# AppImage
if(MCPI_IS_APPIMAGE_BUILD)
    set(CPACK_GENERATOR "External")
    set(CPACK_EXTERNAL_ENABLE_STAGING TRUE)
    set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/appimage.cmake")
endif()

# Package
include(CPack)
