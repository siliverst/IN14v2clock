# Use a base image with Debian or Ubuntu
FROM debian:latest

# Update the package repository and install required tools
RUN apt-get update && apt-get install -y \
    sdcc \
    make \
    build-essential \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Set up a workspace directory
WORKDIR /workspace

# Set the container to always run interactively
CMD ["/bin/bash"]
