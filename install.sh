#!/bin/sh

set -e

sudo adduser "$(whoami)" docker || :

sudo cp -r data/. /
