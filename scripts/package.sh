#!/bin/sh

set -e

# Prepare
NAME='minecraft-pi-reborn'
MODE="$(echo "$1" | tr '[:upper:]' '[:lower:]')"
ARCH="$(echo "$2" | tr '[:upper:]' '[:lower:]')"

# Build
./scripts/setup.sh "${MODE}" "${ARCH}" -DMCPI_IS_APPIMAGE_BUILD=ON
./scripts/build.sh "${MODE}" "${ARCH}"

# Package
cd "build/${MODE}-${ARCH}"
rm -f *.AppImage*
cmake --build . --target package

# Copy Output
cp *.AppImage* ../../out
