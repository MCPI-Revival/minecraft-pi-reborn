#!/bin/sh

set -e

# Launch Minecraft
docker-compose -f "${DOCKER_COMPOSE_YML}" run --rm minecraft-pi