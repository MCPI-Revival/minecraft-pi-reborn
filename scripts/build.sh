#!/bin/sh

set -e

docker build ${DOCKER_BUILD_OPTIONS} --tag thebrokenrail/minecraft-pi:latest .
