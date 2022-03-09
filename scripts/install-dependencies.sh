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

# Install
queue_pkg \
    git \
    cmake \
    ninja-build \
    libglfw3 libglfw3-dev \
    libfreeimage3 libfreeimage-dev \
    crossbuild-essential-armhf \
    gcc g++ \
    nodejs \
    libopenal-dev \
    qemu-user

# Install ARM Dependencies
if [ ! -z "${ARM_PACKAGES_SUPPORTED}" ]; then
    queue_pkg \
        libglfw3:armhf libglfw3-dev:armhf \
        libfreeimage3:armhf \
        libopenal-dev:armhf \
        libglfw3:arm64 libglfw3-dev:arm64 \
        libfreeimage3:arm64 \
        libopenal-dev:arm64 \
        crossbuild-essential-arm64
fi

# Install appimagetool Dependencies
queue_pkg \
    python3-pip \
    python3-setuptools \
    patchelf \
    desktop-file-utils \
    libgdk-pixbuf2.0-dev \
    fakeroot \
    strace \
    fuse \
    sed

# Install Queue
sudo apt-get install --no-install-recommends -y ${PKG_QUEUE}

# Download appimagetool
sudo mkdir -p /opt
sudo wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage -O /opt/appimagetool
# Workaround AppImage Issues With Docker
cd /opt; sudo chmod +x ./appimagetool; sed -i '0,/AI\x02/{s|AI\x02|\x00\x00\x00|}' ./appimagetool; sudo ./appimagetool --appimage-extract
sudo mv /opt/squashfs-root /opt/appimagetool.AppDir
sudo ln -s /opt/appimagetool.AppDir/AppRun /usr/local/bin/appimagetool

# Install appimage-builder
sudo pip3 install git+https://github.com/AppImageCrafters/appimage-builder.git
