#!/bin/sh

set -e

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

# Handle Crash
if [ ${RET} -ne 0 ]; then
    zenity --class "${ZENITY_CLASS}" --error --no-wrap --text 'Minecraft: Pi Edition has crashed!\nLogs are located in /tmp/minecraft-pi.\n\nExit Code: '${RET}
    exit ${RET}
fi