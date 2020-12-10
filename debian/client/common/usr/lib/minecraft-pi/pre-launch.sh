#!/bin/sh

set -e

# Create Log Folder
rm -rf /tmp/minecraft-pi
mkdir -p /tmp/minecraft-pi

# Start Logging
LOG="$(mktemp -u)"
mkfifo "${LOG}"
tee /tmp/minecraft-pi/main.log < "${LOG}" &
TEE_PID=$!

# Run
set +e
/usr/lib/minecraft-pi/run.sh > "${LOG}" 2>&1
RET=$?
set -e

# Kill Logging
kill ${TEE_PID}
rm "${LOG}"

# Handle Crash
if [ $RET -ne 0 ]; then
    zenity --class "${ZENITY_CLASS}" --error --no-wrap --text 'Minecraft: Pi Edition has crashed!\nLogs are located in /tmp/minecraft-pi.'
    exit $RET
fi