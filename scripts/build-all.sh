#!/bin/sh

set -e

# Clean Prefix
rm -rf out

# Build
./scripts/build.sh native client
./scripts/build.sh native server
./scripts/build.sh arm64 client
./scripts/build.sh arm64 server
./scripts/build.sh arm client
./scripts/build.sh arm server
