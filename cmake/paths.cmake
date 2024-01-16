# Specify Installation Paths
set(MCPI_INSTALL_DIR "lib/${MCPI_VARIANT_NAME}")
set(MCPI_BIN_DIR "${MCPI_INSTALL_DIR}/bin")
set(MCPI_LEGAL_DIR "${MCPI_INSTALL_DIR}/legal") # For Software Licenses
set(MCPI_SDK_DIR "${MCPI_INSTALL_DIR}/sdk")
set(MCPI_SDK_LIB_DIR "${MCPI_SDK_DIR}/lib")
set(MCPI_SDK_INCLUDE_DIR "${MCPI_SDK_DIR}/include")

# Library Directory
set(MCPI_LIB_DIR "${MCPI_INSTALL_DIR}/lib")
if(BUILD_ARM_COMPONENTS)
    string(APPEND MCPI_LIB_DIR "/arm")
elseif(BUILD_NATIVE_COMPONENTS)
    string(APPEND MCPI_LIB_DIR "/native")
endif()

# Share Directory
set(MCPI_SHARE_DIR "share")
if(MCPI_IS_APPIMAGE_BUILD)
    string(PREPEND MCPI_SHARE_DIR "usr/")
endif()

# Specify Default Installation Prefix
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(DEFAULT_PREFIX "/usr")
    if(MCPI_IS_APPIMAGE_BUILD)
        set(DEFAULT_PREFIX "/")
    elseif(MCPI_IS_FLATPAK_BUILD)
        set(DEFAULT_PREFIX "/app")
    endif()
    set(CMAKE_INSTALL_PREFIX "${DEFAULT_PREFIX}" CACHE PATH "" FORCE)
    set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT FALSE)
endif()
