#!/bin/sh

set -e

# Don't Use Sudo When Running As Root
if [ "$(id -u)" -eq 0 ]; then
    sudo() {
        "$@"
    }
fi

# Main Script
run() {
    # Add ARM Repository
    for arch in "$@"; do
        sudo dpkg --add-architecture "$(echo "${arch}" | tr '[:upper:]' '[:lower:]')"
    done

    # Update APT
    sudo apt-get update
    sudo apt-get dist-upgrade -y

    # Install Everything In One Go
    PKG_QUEUE=''
    queue_pkg() {
        PKG_QUEUE="${PKG_QUEUE} $@"
    }

    # Build System
    queue_pkg \
        git \
        cmake \
        ninja-build \
        python3 \
        python3-venv

    # Host Dependencies Needed For Compile
    queue_pkg \
        libwayland-bin

    # Architecture-Specific Dependencies
    architecture_specific_pkg() {
        # Compiler
        queue_pkg crossbuild-essential-$1

        # Dependencies
        queue_pkg \
            libopenal-dev:$1

        # GLFW Dependencies
        queue_pkg \
            libwayland-dev:$1 \
            libxkbcommon-dev:$1 \
            libx11-dev:$1 \
            libxcursor-dev:$1 \
            libxi-dev:$1 \
            libxinerama-dev:$1 \
            libxrandr-dev:$1 \
            libxext-dev:$1

        # Zenity Dependencies
        queue_pkg \
            libgtk-3-dev:$1 \
            libglib2.0-dev:$1
    }
    for arch in "$@"; do
        architecture_specific_pkg "$(echo "${arch}" | tr '[:upper:]' '[:lower:]')"
    done

    # AppStream Verification
    queue_pkg \
        appstream

    # Install Queue
    sudo apt-get install --no-install-recommends -y ${PKG_QUEUE}

    # Install appimagetool
    sudo rm -rf /opt/squashfs-root /opt/appimagetool.AppDir 
    sudo rm -f /opt/appimagetool /usr/local/bin/appimagetool
    case "$(dpkg-architecture -qDEB_BUILD_ARCH)" in
        'armhf') APPIMAGE_ARCH='armhf';;
        'arm64') APPIMAGE_ARCH='aarch64';;
        'i386') APPIMAGE_ARCH='i686';;
        'amd64') APPIMAGE_ARCH='x86_64';;
    esac
    sudo mkdir -p /opt
    sudo wget -O /opt/appimagetool "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-${APPIMAGE_ARCH}.AppImage"
    sudo chmod +x /opt/appimagetool
    # Workaround AppImage Issues With Docker
    cd /opt
    sudo sed -i '0,/AI\x02/{s|AI\x02|\x00\x00\x00|}' ./appimagetool
    # Extract
    sudo ./appimagetool --appimage-extract
    sudo rm -f ./appimagetool
    # Link
    sudo mv ./squashfs-root ./appimagetool.AppDir
    sudo ln -s /opt/appimagetool.AppDir/AppRun /usr/local/bin/appimagetool
}

# Run
if [ "$#" -lt 1 ]; then
    run "$(dpkg-architecture -qDEB_BUILD_ARCH)"
else
    run "$@"
fi
