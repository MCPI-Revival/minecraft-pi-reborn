#!/bin/sh

set -e

virgl_test_server &
PID="$!"

xhost local:root

sudo docker-compose up
sudo docker-compose down

kill "${PID}"
