# Download AppImage Runtime
set(RUNTIME_ARCH "unknown")
if(CPACK_MCPI_ARCH STREQUAL "armhf")
    set(RUNTIME_ARCH "armhf")
elseif(CPACK_MCPI_ARCH STREQUAL "arm64")
    set(RUNTIME_ARCH "aarch64")
elseif(CPACK_MCPI_ARCH STREQUAL "amd64")
    set(RUNTIME_ARCH "x86_64")
endif()
set(RUNTIME "${CPACK_TOPLEVEL_DIRECTORY}/runtime")
file(DOWNLOAD
    "https://github.com/AppImage/type2-runtime/releases/download/continuous/runtime-${RUNTIME_ARCH}"
    "${RUNTIME}"
    STATUS DOWNLOAD_STATUS
)
list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
if(NOT STATUS_CODE EQUAL 0)
    message(FATAL_ERROR "Unable To Download AppImage Runtime: ${ERROR_MESSAGE}")
else()
    message(STATUS "Downloaded AppImage Runtime: ${RUNTIME}")
endif()

# Package
set(APPIMAGE_ARCH "unknown")
if(CPACK_MCPI_ARCH STREQUAL "armhf")
    set(APPIMAGE_ARCH "arm")
elseif(CPACK_MCPI_ARCH STREQUAL "arm64")
    set(APPIMAGE_ARCH "arm_aarch64")
elseif(CPACK_MCPI_ARCH STREQUAL "amd64")
    set(APPIMAGE_ARCH "x86_64")
endif()
execute_process(
    COMMAND
        "${CMAKE_COMMAND}" "-E" "env"
        "ARCH=${APPIMAGE_ARCH}"
        "VERSION=${CPACK_MCPI_VERSION}"
        "appimagetool"
        "--updateinformation" "zsync|${CPACK_MCPI_REPO}/releases/download/latest/${CPACK_PACKAGE_FILE_NAME_ZSYNC}${CPACK_MCPI_APPIMAGE_ZSYNC_EXT}"
        "--runtime-file" "${RUNTIME}"
        "--comp" "zstd"
        "${CPACK_TEMPORARY_DIRECTORY}"
        "${CPACK_PACKAGE_FILE_NAME}${CPACK_MCPI_APPIMAGE_EXT}"
    WORKING_DIRECTORY "${CPACK_PACKAGE_DIRECTORY}"
    COMMAND_ERROR_IS_FATAL ANY
)

# Rename ZSync File
file(RENAME "${CPACK_PACKAGE_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}${CPACK_MCPI_APPIMAGE_ZSYNC_EXT}" "${CPACK_PACKAGE_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME_ZSYNC}${CPACK_MCPI_APPIMAGE_ZSYNC_EXT}")

# Summary Message
function(check_file name)
    if(EXISTS "${CPACK_PACKAGE_DIRECTORY}/${name}")
        message(STATUS "Generated: ${name}")
    else()
        message(FATAL_ERROR "Missing File: ${name}")
    endif()
endfunction()
check_file("${CPACK_PACKAGE_FILE_NAME}${CPACK_MCPI_APPIMAGE_EXT}")
check_file("${CPACK_PACKAGE_FILE_NAME_ZSYNC}${CPACK_MCPI_APPIMAGE_ZSYNC_EXT}")
