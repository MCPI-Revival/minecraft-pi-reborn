#!/bin/sh

set -e

# Check Root
if [ ! "$(id -u)" = '0' ]; then
    echo 'Must Run As Root' 1>&2
    exit 1
fi

# Create User Groups
if [ -z "${USER_GID+x}" ]; then
    USER_GID='1000'
fi
groupadd --force --gid "${USER_GID}" user

# Create User
if [ -z "${USER_UID+x}" ]; then
    USER_UID='1000'
fi
useradd --shell /bin/sh --home-dir /home --no-create-home --uid "${USER_UID}" --gid "${USER_GID}" user

# Add Other Groups
if [ ! -z "${USER_OTHER_GIDS+x}" ]; then
    for gid in ${USER_OTHER_GIDS}; do
        groupadd --force --gid "${gid}" "group-${gid}"
        usermod -aG "${gid}" user
    done
fi

# Start
exec gosu "${USER_UID}" ./launcher