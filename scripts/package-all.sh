#!/bin/sh

set -e

# Clean Prefix
rm -rf out

# Build
./scripts/package.sh client amd64
./scripts/package.sh server amd64
./scripts/package.sh client arm64
./scripts/package.sh server arm64
./scripts/package.sh client armhf
./scripts/package.sh server armhf
