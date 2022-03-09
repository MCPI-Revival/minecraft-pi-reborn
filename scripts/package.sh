#!/bin/sh

set -e

# Generate
./scripts/generate-appimage-builder-yaml.js "$1" "$2"

# Build/Package
appimage-builder --recipe AppImageBuilder.yml
