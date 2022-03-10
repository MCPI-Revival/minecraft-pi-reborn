FROM buildpack-deps:bullseye

# Setup
ENV ARM_PACKAGES_SUPPORTED=1

# Install
ADD ./scripts/install-dependencies.sh /
RUN \
    apt-get update && \
    apt-get install --no-install-recommends -y sudo && \
    /install-dependencies.sh && \
    rm -rf /var/lib/apt/lists/*
