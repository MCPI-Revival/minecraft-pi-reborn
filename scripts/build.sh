#!/bin/sh

set -e

docker build ${DOCKER_BUILD_OPTIONS} --tag thebrokenrail/minecraft-pi:client -f Dockerfile.client .
#docker build ${DOCKER_BUILD_OPTIONS} --tag thebrokenrail/minecraft-pi:server -f Dockerfile.server .
