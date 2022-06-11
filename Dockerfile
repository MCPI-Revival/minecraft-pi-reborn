FROM debian:bullseye-slim

# Install
RUN \
    apt-get update && \
    apt-get install -y tini sed patchelf qemu-user && \
    apt-get --fix-broken install -y && \
    rm -rf /var/lib/apt/lists/*

# Copy
ADD ./out/server-amd64 /app

# Setup Working Directory
RUN mkdir /data
WORKDIR /data

# Setup Entrypoint
ENTRYPOINT ["/usr/bin/tini", "--"]
CMD ["/app/usr/bin/minecraft-pi-reborn-server"]
