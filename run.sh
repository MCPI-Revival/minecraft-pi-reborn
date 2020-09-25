#!/bin/sh

set -e

virgl_test_server &
PID="$!"

xhost local:root

sudo docker run -it -v /tmp/.X11-unix:/tmp/.X11-unix -v /tmp/.virgl_test:/tmp/.virgl_test -v ~/.minecraft-pi:/root/.minecraft -e DISPLAY=unix${DISPLAY} thebrokenrail/minecraft-pi

kill "${PID}"
