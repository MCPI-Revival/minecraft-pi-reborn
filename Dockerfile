FROM debian:bullseye-slim

# Install
RUN \
    apt-get update && \
    apt-get install -y tini sed && \
    apt-get --fix-broken install -y && \
    rm -rf /var/lib/apt/lists/*

# Copy AppImage
RUN mkdir /app
ADD ./out/minecraft-pi-reborn-server-*-amd64.AppImage /app

# Extract AppImage
WORKDIR /app
RUN \
    sed -i '0,/AI\x02/{s|AI\x02|\x00\x00\x00|}' ./*.AppImage && \
    ./*.AppImage --appimage-extract && \
    rm -f ./*.AppImage

# Setup AppImage
ENV OWD=/data
ENV APPDIR=/app/squashfs-root

# Setup Working Directory
RUN mkdir /data
WORKDIR /data

# Setup Entrypoint
ENTRYPOINT ["/usr/bin/tini", "--"]
CMD ["/app/squashfs-root/AppRun"]
