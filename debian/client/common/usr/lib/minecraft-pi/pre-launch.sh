#!/bin/sh

set -e

# Prepare Data Folder
mkdir -p "${USER_HOME}/.minecraft-pi"

# Create Log Folder
rm -rf /tmp/minecraft-pi
mkdir -p /tmp/minecraft-pi

# Start Logging
touch /tmp/minecraft-pi/main.log
tail -f /tmp/minecraft-pi/main.log &
TAIL_PID=$!

# Run
set +e
/usr/lib/minecraft-pi/run.sh > /tmp/minecraft-pi/main.log 2>&1
RET=$?
set -e

# Kill Logging
kill ${TAIL_PID}

# Exit
exit ${RET}