#!/bin/sh

set -e

# Clean Prefix
rm -rf out

# Build
./scripts/build.sh client x86_64
./scripts/build.sh server x86_64
./scripts/build.sh client arm64
./scripts/build.sh server arm64
./scripts/build.sh client arm
./scripts/build.sh server arm
