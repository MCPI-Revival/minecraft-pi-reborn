FROM buildpack-deps:buster

# Install Updated CMakes
RUN \
    echo 'deb http://deb.debian.org/debian buster-backports main' > /etc/apt/sources.list.d/backports.list && \
    apt-get update && \
    apt-get install --no-install-recommends -y -t buster-backports cmake && \
    rm -rf /var/lib/apt/lists/*

# Install
ADD ./scripts/install-dependencies.sh /
RUN \
    apt-get update && \
    apt-get install --no-install-recommends -y sudo && \
    /install-dependencies.sh amd64 armhf arm64 && \
    rm -rf /var/lib/apt/lists/*
