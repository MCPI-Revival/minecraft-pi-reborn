#!/bin/sh

set -e

if [ ! -z "${MCPI_CUSTOM_APT_REPO}" ]; then
    echo "${MCPI_CUSTOM_APT_REPO}"
    exit 0
fi

ID="$(sed -n -e 's/^ID=//p' /etc/os-release)"
ID_LIKE="$(sed -n -e 's/^ID_LIKE=//p' /etc/os-release)"
VERSION_CODENAME="$(sed -n -e 's/^VERSION_CODENAME=//p' /etc/os-release)"

OUT=""
get_apt_sources() {
    if [ "${1}" = "ubuntu" ]; then
        OUT="deb [arch=i386,amd64] http://archive.ubuntu.com/ubuntu/ ${VERSION_CODENAME} main restricted
deb [arch=i386,amd64] http://archive.ubuntu.com/ubuntu/ ${VERSION_CODENAME}-updates main restricted
deb [arch=i386,amd64] http://archive.ubuntu.com/ubuntu/ ${VERSION_CODENAME} universe
deb [arch=i386,amd64] http://archive.ubuntu.com/ubuntu/ ${VERSION_CODENAME}-updates universe
deb [arch=armhf,arm64] http://ports.ubuntu.com/ubuntu-ports/ ${VERSION_CODENAME} main restricted
deb [arch=armhf,arm64] http://ports.ubuntu.com/ubuntu-ports/ ${VERSION_CODENAME}-updates main restricted
deb [arch=armhf,arm64] http://ports.ubuntu.com/ubuntu-ports/ ${VERSION_CODENAME} universe
deb [arch=armhf,arm64] http://ports.ubuntu.com/ubuntu-ports/ ${VERSION_CODENAME}-updates universe"
    elif [ "${1}" = "debian" ]; then
        OUT="deb https://deb.debian.org/debian/ ${VERSION_CODENAME} main"
        if [ "${VERSION_CODENAME}" != "sid" ] && [ "${VERSION_CODENAME}" != "unstable" ] && [ "${VERSION_CODENAME}" != "experimental" ]; then
            OUT="${OUT}
deb https://deb.debian.org/debian/ ${VERSION_CODENAME}-updates main"
        fi
    fi
}
get_apt_sources "${ID}"
if [ -z "${OUT}" ]; then
    get_apt_sources "${ID_LIKE}"
fi

if [ ! -z "${OUT}" ]; then
    echo "${OUT}"
else
    echo "Unsupported Distribution: ${ID}" 1>&2
    exit 1
fi
