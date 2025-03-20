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
export MCPI_PROFILE_DIRECTORY="$(pwd)/.testing-tmp"
rm -rf "${MCPI_PROFILE_DIRECTORY}"
mkdir "${MCPI_PROFILE_DIRECTORY}"

# Prepare AppImage For Docker
EXE="$(mktemp)"
cp "${APPIMAGE}" "${EXE}"
./scripts/fix-appimage-for-docker.sh "${EXE}"

# Run
if [ "${MODE}" = "server" ]; then
    # Server Test
    "${EXE}" --appimage-extract-and-run --server --only-generate
else
    # Client Test
    "${EXE}" --appimage-extract-and-run --default --no-cache --benchmark --force-headless
fi

# Clean Up
rm -f "${EXE}"
