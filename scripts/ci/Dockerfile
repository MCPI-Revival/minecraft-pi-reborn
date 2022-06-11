FROM buildpack-deps:bullseye

# Install
ADD ./scripts/install-dependencies.sh /
RUN \
    apt-get update && \
    apt-get install --no-install-recommends -y sudo && \
    /install-dependencies.sh amd64 armhf arm64 && \
    rm -rf /var/lib/apt/lists/*
