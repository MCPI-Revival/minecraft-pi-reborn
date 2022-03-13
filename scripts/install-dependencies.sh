#!/bin/sh

set -e

# This Script Assumes An x86_64 Host
if [ "$(uname -m)" != "x86_64" ]; then
    echo 'Invalid Build Architecture'
    exit 1
fi

# Add ARM Repository
if [ ! -z "${ARM_PACKAGES_SUPPORTED}" ]; then
    sudo dpkg --add-architecture armhf
    sudo dpkg --add-architecture arm64
fi

# Update APT
sudo apt-get update
sudo apt-get dist-upgrade -y

# Install Everything In One Go
PKG_QUEUE=''
queue_pkg() {
    PKG_QUEUE="${PKG_QUEUE} $@"
}

# Build Tools
queue_pkg \
    git \
    cmake \
    ninja-build \
    crossbuild-essential-armhf \
    gcc g++ \
    nodejs

# Dependencies
queue_pkg \
    libfreeimage3 libfreeimage-dev \
    libopenal-dev \
    qemu-user

# GLFW Dependencies
queue_pkg \
    libwayland-dev \
    libxkbcommon-dev \
    wayland-protocols \
    libx11-dev \
    libxcursor-dev \
    libxi-dev \
    libxinerama-dev \
    libxrandr-dev \
    libxext-dev

# Zenity Dependencies
queue_pkg \
    libgtk-3-dev \
    libglib2.0-dev

# ARM Packages
if [ ! -z "${ARM_PACKAGES_SUPPORTED}" ]; then
    # Build Tools
    queue_pkg \
        crossbuild-essential-arm64

    # Dependencies
    queue_pkg \
        libfreeimage3:armhf libfreeimage3:arm64 \
        libopenal-dev:armhf libopenal-dev:arm64

    # GLFW Dependencies
    queue_pkg \
        libwayland-dev:armhf libwayland-dev:arm64 \
        libxkbcommon-dev:armhf libxkbcommon-dev:arm64 \
        libx11-dev:armhf libx11-dev:arm64 \
        libxcursor-dev:armhf libxcursor-dev:arm64 \
        libxi-dev:armhf libxi-dev:arm64 \
        libxinerama-dev:armhf libxinerama-dev:arm64 \
        libxrandr-dev:armhf libxrandr-dev:arm64 \
        libxext-dev:armhf libxext-dev:arm64

    # Zenity Dependencies
    queue_pkg \
        libgtk-3-dev:armhf libgtk-3-dev:arm64 \
        libglib2.0-dev:armhf libglib2.0-dev:arm64
fi

# Install appimagetool & appimage-builder Dependencies
queue_pkg \
    python3-pip \
    python3-setuptools \
    python3-wheel \
    patchelf \
    desktop-file-utils \
    libgdk-pixbuf2.0-dev \
    fakeroot \
    strace \
    fuse \
    gtk-update-icon-cache \
    shared-mime-info \
    sed

# Install Queue
sudo apt-get install --no-install-recommends -y ${PKG_QUEUE}

# Download appimagetool
sudo mkdir -p /opt
sudo wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage -O /opt/appimagetool
sudo chmod +x /opt/appimagetool
# Workaround AppImage Issues With Docker
cd /opt
sudo sed -i '0,/AI\x02/{s|AI\x02|\x00\x00\x00|}' ./appimagetool
sudo rm -rf /opt/squashfs-root /opt/appimagetool.AppDir
sudo ./appimagetool --appimage-extract
sudo rm -f ./appimagetool
sudo mv /opt/squashfs-root /opt/appimagetool.AppDir
sudo rm -f /usr/local/bin/appimagetool
sudo ln -s /opt/appimagetool.AppDir/AppRun /usr/local/bin/appimagetool

# Install appimage-builder
sudo pip3 install 'git+https://github.com/TheBrokenRail/appimage-builder.git@combined'
