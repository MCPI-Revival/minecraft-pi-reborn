#!/bin/sh

set -e

# Patch File
FILE="$1"
chmod +x "${FILE}"
sed -i '0,/AI\x02/{s|AI\x02|\x00\x00\x00|}' "${FILE}"