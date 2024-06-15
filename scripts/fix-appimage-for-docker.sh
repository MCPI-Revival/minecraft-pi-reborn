#!/bin/sh

exec sed -i '0,/AI\x02/{s|AI\x02|\x00\x00\x00|}' "$1"