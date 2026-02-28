# Specify Installation Paths
set(MCPI_INSTALL_DIR "lib/${MCPI_APP_NAME}")
set(MCPI_LEGAL_DIR "${MCPI_INSTALL_DIR}/legal") # For Software Licenses
set(MCPI_SDK_DIR "${MCPI_INSTALL_DIR}/sdk")
set(MCPI_SDK_LIB_DIR "${MCPI_SDK_DIR}/lib")
set(MCPI_SDK_INCLUDE_DIR "${MCPI_SDK_DIR}/include")

# Library Directory
set(MCPI_LIB_DIR "${MCPI_INSTALL_DIR}/lib")
set(MCPI_LIB_DIR_ARM "${MCPI_LIB_DIR}/arm")
if(BUILD_ARM_COMPONENTS)
    set(MCPI_LIB_DIR "${MCPI_LIB_DIR_ARM}")
elseif(BUILD_NATIVE_COMPONENTS)
    if(MCPI_WIN32)
        # Windows Requires DLLs To Be
        # Placed Alongside Executables
        set(MCPI_LIB_DIR "${MCPI_INSTALL_DIR}")
    else()
        string(APPEND MCPI_LIB_DIR "/native")
    endif()
endif()

# Share Directory
set(MCPI_SHARE_DIR "share")
if(MCPI_IS_APPIMAGE_BUILD)
    string(PREPEND MCPI_SHARE_DIR "usr/")
elseif(MCPI_IS_ZIP_BUILD)
    string(PREPEND MCPI_SHARE_DIR "${MCPI_INSTALL_DIR}/")
endif()

# Specify Default Installation Prefix
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(DEFAULT_PREFIX "/usr")
    if(MCPI_IS_APPIMAGE_BUILD OR MCPI_WIN32)
        set(DEFAULT_PREFIX "/")
    elseif(MCPI_IS_FLATPAK_BUILD)
        set(DEFAULT_PREFIX "/app")
    endif()
    force_set(CMAKE_INSTALL_PREFIX "${DEFAULT_PREFIX}" PATH)
    set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT FALSE)
endif()
