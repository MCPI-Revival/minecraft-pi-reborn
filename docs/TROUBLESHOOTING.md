# Troubleshooting
Game logs are located in ``/tmp/minecraft-pi``.

## ``Couldn't connect to Docker daemon at http+docker://localhost - is it running?``
Start Docker if it isn't running:
```sh
sudo service docker start
```

## ``Error response from daemon: error gathering device information while adding custom device "/dev/dri": no such file or directory``
Make sure you are using the correct GPU drivers for your system.

If you are using a Raspberry Pi, make sure your GPU driver is set to ``Full KMS`` or ``Fake KMS`` in ``raspi-config``.

## ``Segmentation Fault`` (Exit Code: ``139``)
Report an issue with reproduction instructions and system details.

## ``[ERR]: Invalid ~/.minecraft-pi Permissions``
Update ``~/.minecraft-pi`` permissions:
```sh
sudo chown -R "$(id -u):$(id -g)" ~/.minecraft-pi
chmod -R u+rw ~/.minecraft-pi
```

## Other
If you experience a crash/error not listed above, report it on the issue tracker **with your game log attached**.