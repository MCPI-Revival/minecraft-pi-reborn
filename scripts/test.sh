#!/bin/sh

set -e

# Change Directory
cd "$(dirname "$0")/../"

# Variables
MODE="$(echo "$1" | tr '[:upper:]' '[:lower:]')"
ARCH="$(echo "$2" | tr '[:upper:]' '[:lower:]')"
APPIMAGE="$(pwd)/out/minecraft-pi-reborn-$(cat VERSION)-${ARCH}.AppImage"

# Check If File Exists
if [ ! -f "${APPIMAGE}" ]; then
    echo 'Missing AppImage!' > /dev/stderr
    exit 1
fi

# Make Test Directory
TEST_WORKING_DIR="$(pwd)/.testing-tmp"
rm -rf "${TEST_WORKING_DIR}"
mkdir "${TEST_WORKING_DIR}"
ROOT="$(pwd)"
cd "${TEST_WORKING_DIR}"

# Prepare AppImage For Docker
cp "${APPIMAGE}" tmp.AppImage
"${ROOT}/scripts/fix-appimage-for-docker.sh" tmp.AppImage
chmod +x tmp.AppImage

# Run
if [ "${MODE}" = "server" ]; then
    # Server Test
    ./tmp.AppImage --appimage-extract-and-run --server --only-generate
else
    # Client Test
    export MCPI_PROFILE_DIRECTORY="${TEST_WORKING_DIR}"
    ./tmp.AppImage --appimage-extract-and-run --default --no-cache --benchmark --force-headless
fi

# Clean Up
rm -rf "${TEST_WORKING_DIR}"
