#!/bin/sh

set -e

# Setup
export XDG_SESSION_TYPE=x11
unset MCPI_GUI_SCALE
export PATH="$(pwd)/out/host/usr/bin:${PATH}"

# Setup Feature Flags
export MCPI_FEATURE_FLAGS="$(
    # Get All Feature Flags
    minecraft-pi-reborn --print-available-feature-flags |
    # Find Enabled Feature Flags
    grep '^TRUE ' | cut -f2- -d' ' |
    # Disable Flags
    grep -v 'Add Welcome Screen' |
    grep -v 'Improved Cursor Rendering' |
    # Format
    tr '\n' '|'
)"

# Run
minecraft-pi-reborn --default --no-cache &
PID="$!"

# Screenshot
sleep 3
gnome-screenshot --window --file=images/start.png

# Kill
kill "${PID}"
wait "${PID}"
