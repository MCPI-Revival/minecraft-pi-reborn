#!/bin/sh

set -e

# Start VirGL
virgl_test_server > /tmp/virgl.log 2>&1 &
VIRGL_PID="$!"

# Launch Minecraft
${DOCKER_COMPOSE} run --rm minecraft-pi || :
RET="$?"

# Kill VirGL
kill "${VIRGL_PID}"

exit "${RET}"