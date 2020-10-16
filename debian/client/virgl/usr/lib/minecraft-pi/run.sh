#!/bin/sh

set -e

# Enable VirGL
export MCPI_MODE=virgl

# Start VirGL
virgl_test_server &
VIRGL_PID="$!"

# Launch Minecraft
${DOCKER_COMPOSE} run --rm  minecraft-pi || :
RET="$?"

# Kill VirGL
kill "${VIRGL_PID}"

exit "${RET}"