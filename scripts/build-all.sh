#!/bin/sh

set -e

# Clean Prefix
rm -rf out

# Build
./scripts/build.sh client amd64
./scripts/build.sh server amd64
./scripts/build.sh client arm64
./scripts/build.sh server arm64
./scripts/build.sh client armhf
./scripts/build.sh server armhf
