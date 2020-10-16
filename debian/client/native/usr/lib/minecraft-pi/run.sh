#!/bin/sh

set -e

# Enable VirGL
export MCPI_MODE=native

# Launch Minecraft
${DOCKER_COMPOSE} run --rm  minecraft-pi