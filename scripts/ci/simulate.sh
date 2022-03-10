#!/bin/sh

set -e

# Build Docker Image
docker build -f Dockerfile.build -t minecraft-pi-reborn-build .

# Run
docker run --rm -v "$(pwd):/data" -w '/data' -u '1000:1000' minecraft-pi-reborn-build ./scripts/ci/run.sh
