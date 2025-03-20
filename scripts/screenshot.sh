#!/bin/sh

set -e

# Change Directory
cd "$(dirname "$0")/../"

# Setup
export XDG_SESSION_TYPE=x11
unset MCPI_GUI_SCALE
unset MCPI_USERNAME
export PATH="$(pwd)/out/none/host/usr/bin:${PATH}"

# Game Directory
export MCPI_PROFILE_DIRECTORY="$(pwd)/.testing-tmp"
rm -rf "${MCPI_PROFILE_DIRECTORY}"
mkdir "${MCPI_PROFILE_DIRECTORY}"

# Take Screenshot
screenshot() {
    # Arguments
    IMAGE="images/screenshots/$1.png"
    TIMER="$2"
    shift 2

    # Run
    minecraft-pi-reborn "$@" &
    PID="$!"

    # Screenshot
    sleep "${TIMER}"
    gnome-screenshot --window "--file=${IMAGE}"

    # Kill
    kill "${PID}"
    wait "${PID}" || :
}

# Launcher
screenshot launcher 0.5

# Start Screen
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
screenshot start 3 --default