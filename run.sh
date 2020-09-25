#!/bin/sh

set -e

virgl_test_server &
PID="$!"

xhost local:root

sudo docker-compose up

kill "${PID}"
