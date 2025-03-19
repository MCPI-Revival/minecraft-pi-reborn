cmake_minimum_required(VERSION 3.25.0)
project(tools)

# Utility Functions
include(../cmake/util/util.cmake)

# Options
force_set(BUILD_TESTING FALSE BOOL)
force_set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/bin" FILEPATH)
force_set(CMAKE_INSTALL_BINDIR "." FILEPATH)

# Ninja
include(FetchContent)
FetchContent_Declare(ninja
    GIT_REPOSITORY "https://github.com/digit-google/ninja.git"
    GIT_TAG jobserver
)
FetchContent_MakeAvailable(ninja)

# Make
include(ExternalProject)
ExternalProject_Add(make
    URL "https://ftp.gnu.org/gnu/make/make-4.4.1.tar.gz"
    URL_HASH "SHA256=dd16fb1d67bfab79a72f5e8390735c49e3e8e70b4945a15ab1f81ddb78658fb3"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    CONFIGURE_COMMAND
        "<SOURCE_DIR>/configure"
        "--prefix=<INSTALL_DIR>"
        "--without-guile"
        "--enable-silent-rules"
        "--quiet"
        "CFLAGS=${CMAKE_C_FLAGS}"
    BUILD_IN_SOURCE TRUE
    INSTALL_COMMAND ""
)
ExternalProject_Get_Property(make SOURCE_DIR)
install(PROGRAMS "${SOURCE_DIR}/make" DESTINATION "${CMAKE_INSTALL_BINDIR}")

# Download AppImageTool
execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
if(ARCH STREQUAL "armv8b" OR ARCH STREQUAL "armv8l")
    set(ARCH "aarch64")
endif()
set(APPIMAGETOOL_NAME "appimagetool")
set(APPIMAGETOOL_BIN "${CMAKE_CURRENT_BINARY_DIR}/${APPIMAGETOOL_NAME}.AppImage")
safe_download(
    "AppImageTool"
    "https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-${ARCH}.AppImage"
    "${APPIMAGETOOL_BIN}"
)
# Fix
execute_process(
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../scripts/fix-appimage-for-docker.sh" "${APPIMAGETOOL_BIN}"
    COMMAND_ERROR_IS_FATAL ANY
)
# Install
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${APPIMAGETOOL_NAME}"
    "#!/bin/sh\n"
    "exec \"${APPIMAGETOOL_BIN}\" --appimage-extract-and-run \"\$@\"\n"
)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${APPIMAGETOOL_NAME}" DESTINATION "${CMAKE_INSTALL_BINDIR}")