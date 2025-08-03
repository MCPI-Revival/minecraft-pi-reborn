#!/bin/sh

set -e

# End Process
end() {
    kill "$1"
    wait "$1" || :
}

# Setup Nested X11
NESTED_DISPLAY=':99'
Xephyr "${NESTED_DISPLAY}" -ac > /dev/null 2>&1 &
X11_PID="$!"
sleep 1
mutter --x11 --display "${NESTED_DISPLAY}" --sm-disable > /dev/null 2>&1 &
SHELL_PID="$!"
sleep 1
export DISPLAY="${NESTED_DISPLAY}"

# Change Directory
cd "$(dirname "$0")/../../"

# Setup
export XDG_SESSION_TYPE=x11
unset MCPI_GUI_SCALE
unset MCPI_USERNAME
PWD="$(pwd)"
export PATH="${PWD}/out/host/usr/bin:${PATH}"

# Game Directory
export MCPI_PROFILE_DIRECTORY="${PWD}/.testing-tmp"
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
    FOCUSED="$(xdotool getwindowfocus)"
    TARGET="$(xwininfo -int -children -id "${FOCUSED}" | awk '/Parent window id:/{print $4}')"
    maim --quiet --hidecursor --window "${TARGET}" "${IMAGE}"

    # Kill
    end "${PID}"
}

# Launcher
screenshot launcher 0.5

# Start Screen
export MCPI_PROMOTIONAL=
screenshot start 3 --default

# Kill Shell
end "${SHELL_PID}"
end "${X11_PID}"