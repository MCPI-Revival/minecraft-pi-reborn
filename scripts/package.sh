#!/bin/sh

set -e

# Prepare
NAME='minecraft-pi-reborn'

# Build
./scripts/setup.sh "$1" "$2" -DMCPI_IS_APPIMAGE_BUILD=ON
./scripts/build.sh "$1" "$2"

# Download Runtime
case "$2" in
    'armhf') RUNTIME_ARCH='armhf';;
    'arm64') RUNTIME_ARCH='aarch64';;
    'i386') RUNTIME_ARCH='i686';;
    'amd64') RUNTIME_ARCH='x86_64';;
esac
mkdir -p build/appimage
if [ ! -f "build/appimage/runtime-$2" ]; then
    wget -O "build/appimage/runtime-$2" "https://github.com/AppImage/AppImageKit/releases/download/continuous/runtime-${RUNTIME_ARCH}"
fi

# Package
ARCH="${RUNTIME_ARCH}" appimagetool \
    --updateinformation "zsync|https://jenkins.thebrokenrail.com/job/minecraft-pi-reborn/job/master/lastSuccessfulBuild/artifact/out/${NAME}-latest-$2.AppImage.zsync" \
    --runtime-file "build/appimage/runtime-$2" \
    --comp xz \
    "./out/$1-$2" \
    "./out/${NAME}-$1-$(cat VERSION)-$2.AppImage"

# Move ZSync
rm -f "./out/${NAME}-$1-latest-$2.AppImage.zsync"
mv "./${NAME}-$1-$(cat VERSION)-$2.AppImage.zsync" "./out/${NAME}-$1-latest-$2.AppImage.zsync"
