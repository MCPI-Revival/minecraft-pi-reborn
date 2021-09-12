FROM debian:bullseye-slim

# Copy DEB
ADD ./out/minecraft-pi-reborn-server_*_amd64.deb /root

# Install
RUN \
    apt-get update && \
    apt-get install -y tini && \
    (dpkg -i /root/*.deb || :) && \
    apt-get --fix-broken install -y && \
    rm -f /root/*.deb && \
    rm -rf /var/lib/apt/lists/*

# Setup Working Directory
RUN mkdir /data
WORKDIR /data

# Setup Entrypoint
ENTRYPOINT ["/usr/bin/tini", "--"]
CMD ["minecraft-pi-reborn-server"]
