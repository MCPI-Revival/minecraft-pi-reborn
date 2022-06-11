#!/bin/sh

set -e

# Main Script
run() {
    # Add ARM Repository
    for arch in "$@"; do
        sudo dpkg --add-architecture "${arch}"
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
        nodejs

    # Host Dependencies Needed For Compile
    queue_pkg \
        libwayland-bin \
        libfreeimage-dev

    # Host Dependencies Needed For Running
    queue_pkg \
        qemu-user \
        patchelf

    # Architecture-Specific Dependencies
    architecture_specific_pkg() {
        # Compiler
        queue_pkg \
            crossbuild-essential-$1

        # Dependencies
        queue_pkg \
            libfreeimage3:$1 \
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
        architecture_specific_pkg "${arch}"
    done

    # Install appimagetool & appimage-builder Dependencies
    queue_pkg \
        python3-pip \
        python3-setuptools \
        python3-wheel \
        patchelf \
        desktop-file-utils \
        libgdk-pixbuf2.0-dev \
        fakeroot \
        gtk-update-icon-cache \
        shared-mime-info \
        squashfs-tools \
        zsync \
        sed

    # Install Queue
    sudo apt-get install --no-install-recommends -y ${PKG_QUEUE}

    # Install appimage-builder
    sudo pip3 install 'git+https://github.com/AppImageCrafters/appimage-builder.git'
}

# Run
if [ "$#" -lt 1 ]; then
    run "$(dpkg-architecture -qDEB_BUILD_ARCH)"
else
    run "$@"
fi
