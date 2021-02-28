#!/bin/sh

set -e

# Start VirGL
virgl_test_server > /tmp/minecraft-pi/virgl.log 2>&1 &
VIRGL_PID=$!

# Launch Minecraft
set +e
docker-compose -f "${DOCKER_COMPOSE_YML}" run --rm minecraft-pi
RET=$?
set -e

# Kill VirGL
kill ${VIRGL_PID} > /dev/null 2>&1 || :

# Exit
exit ${RET}