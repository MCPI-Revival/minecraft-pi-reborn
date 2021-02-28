#!/bin/sh

set -e

# Launch Minecraft
exec docker-compose -f "${DOCKER_COMPOSE_YML}" run --rm minecraft-pi