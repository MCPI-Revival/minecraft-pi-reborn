#!/bin/sh

set -e

docker build ${DOCKER_BUILD_OPTIONS} --tag thebrokenrail/minecraft-pi-reborn:client -f Dockerfile.client .
docker build ${DOCKER_BUILD_OPTIONS} --tag thebrokenrail/minecraft-pi-reborn:server -f Dockerfile.server .
