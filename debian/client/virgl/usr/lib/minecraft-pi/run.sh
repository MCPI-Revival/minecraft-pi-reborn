#!/bin/sh

set -e

# Start VirGL
virgl_test_server > /tmp/minecraft-pi/virgl.log 2>&1 &
VIRGL_PID=$!

# Launch Minecraft
set +e
${DOCKER_COMPOSE} run --rm minecraft-pi
RET=$?
set -e

# Kill VirGL
kill ${VIRGL_PID}

exit ${RET}