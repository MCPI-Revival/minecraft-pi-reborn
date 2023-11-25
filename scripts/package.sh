#!/bin/sh

set -e

# Prepare
NAME='minecraft-pi-reborn'
MODE="$(echo "$1" | tr '[:upper:]' '[:lower:]')"
ARCH="$(echo "$2" | tr '[:upper:]' '[:lower:]')"

# Build
./scripts/setup.sh "${MODE}" "${ARCH}" -DMCPI_IS_APPIMAGE_BUILD=ON
./scripts/build.sh "${MODE}" "${ARCH}"

# Download Runtime
mkdir -p build/appimage
if [ ! -f "build/appimage/runtime-${ARCH}" ]; then
    case "${ARCH}" in
        'armhf') RUNTIME_ARCH='armhf';;
        'arm64') RUNTIME_ARCH='aarch64';;
        'i386') RUNTIME_ARCH='i686';;
        'amd64') RUNTIME_ARCH='x86_64';;
    esac
    wget -O "build/appimage/runtime-${ARCH}" "https://github.com/AppImage/AppImageKit/releases/download/continuous/runtime-${RUNTIME_ARCH}"
fi

# Package
case "${ARCH}" in
    'armhf') APPIMAGE_ARCH='arm';;
    'arm64') APPIMAGE_ARCH='arm_aarch64';;
    'i386') APPIMAGE_ARCH='i686';;
    'amd64') APPIMAGE_ARCH='x86_64';;
esac
ARCH="${APPIMAGE_ARCH}" appimagetool \
    --updateinformation "zsync|https://gitea.thebrokenrail.com/minecraft-pi-reborn/minecraft-pi-reborn/releases/download/latest/${NAME}-${MODE}-latest-${ARCH}.AppImage.zsync" \
    --runtime-file "build/appimage/runtime-${ARCH}" \
    --comp xz \
    "./out/${MODE}-${ARCH}" \
    "./out/${NAME}-${MODE}-$(cat VERSION)-${ARCH}.AppImage"

# Move ZSync
rm -f "./out/${NAME}-${MODE}-latest-${ARCH}.AppImage.zsync"
mv "./${NAME}-${MODE}-$(cat VERSION)-${ARCH}.AppImage.zsync" "./out/${NAME}-${MODE}-latest-${ARCH}.AppImage.zsync"
