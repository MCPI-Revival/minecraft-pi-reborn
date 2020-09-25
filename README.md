# Minecraft: Pi Edition For Docker

## Dependencies
```sh
# Required For Hardware Acceleration
sudo apt install virgl-server

# Required For ARM Support
sudo docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
```

## Tutorial
```sh
virgl_test_server &
PID="$!"

xhost local:root

sudo docker run -it -v /tmp/.X11-unix:/tmp/.X11-unix -v /tmp/.virgl_test:/tmp/.virgl_test -v ~/.minecraft-pi:/root/.minecraft -e DISPLAY=unix${DISPLAY} thebrokenrail/minecraft-pi

kill "${PID}"
```

## Tweaks
The included version of Minecraft: Pi Ediiton is slightly modified so it uses the old touchscreen UI.
