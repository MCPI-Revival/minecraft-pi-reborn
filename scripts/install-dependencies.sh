#!/bin/sh

set -e

# Don't Use Sudo When Running As Root
if [ "$(id -u)" -eq 0 ]; then
    sudo() {
        "$@"
    }
fi

# Setup Backports
CODENAME="$(. /etc/os-release && echo "${VERSION_CODENAME}")"
BACKPORTS=''
if [ "${CODENAME}" = 'bullseye' ]; then
    BACKPORTS="${CODENAME}-backports"
    echo "deb http://deb.debian.org/debian ${BACKPORTS} main" | sudo tee "/etc/apt/sources.list.d/${BACKPORTS}.list" > /dev/null
    BACKPORTS="/${BACKPORTS}"
fi

# Variables
MODE="$1"
ARCH="$(echo "$2" | tr '[:upper:]' '[:lower:]')"

# Add ARM Repository
sudo dpkg --add-architecture "${ARCH}"

# Update APT
sudo apt-get update
sudo apt-get dist-upgrade -y

# Run APT
install_pkg() {
    sudo apt-get install --no-install-recommends -y "$@"
}

# Build Dependencies
run_build() {
    install_pkg \
        `# Build System` \
        git \
        "cmake${BACKPORTS}" \
        ninja-build \
        python3 \
        python3-venv \
        "python3-tomli${BACKPORTS}" \
        `# Host Dependencies Needed For Compile` \
        libwayland-bin \
        `# Compiler` \
        "crossbuild-essential-$1" \
        `# Main Dependencies` \
        "libopenal-dev:$1" \
        `# GLFW Dependencies` \
        "libwayland-dev:$1" \
        "libxkbcommon-dev:$1" \
        "libx11-dev:$1" \
        "libxcursor-dev:$1" \
        "libxi-dev:$1" \
        "libxinerama-dev:$1" \
        "libxrandr-dev:$1" \
        "libxext-dev:$1" \
        `# QEMU Dependencies` \
        "libglib2.0-dev:$1" \
        `# ImGui Dependencies` \
        "libglvnd-dev:$1" \
        `# AppStream Verification` \
        appstream

    # Install appimagetool
    sudo rm -rf /opt/squashfs-root /opt/appimagetool /usr/local/bin/appimagetool
    case "$(dpkg --print-architecture)" in
        'armhf') APPIMAGE_ARCH='armhf';;
        'arm64') APPIMAGE_ARCH='aarch64';;
        'i386') APPIMAGE_ARCH='i686';;
        'amd64') APPIMAGE_ARCH='x86_64';;
    esac
    sudo mkdir -p /opt
    sudo wget -O /opt/appimagetool "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-${APPIMAGE_ARCH}.AppImage"
    sudo chmod +x /opt/appimagetool
    # Workaround AppImage Issues With Docker
    sudo ./scripts/fix-appimage-for-docker.sh /opt/appimagetool
    # Extract
    cd /opt
    sudo ./appimagetool --appimage-extract > /dev/null
    sudo rm -f ./appimagetool
    # Link
    sudo mv ./squashfs-root ./appimagetool
    sudo ln -s /opt/appimagetool/AppRun /usr/local/bin/appimagetool
}

# Test Dependencies
run_test() {
    install_pkg \
        "libc6:$1" \
        "libstdc++6:$1" \
        "libopenal1:$1" \
        "libglib2.0-0:$1"
}

# Example Mods Dependencies
run_example_mods() {
    install_pkg \
        cmake \
        ninja-build \
        g++-arm-linux-gnueabihf \
        gcc-arm-linux-gnueabihf
}

# Install Packages
"run_${MODE}" "${ARCH}"
