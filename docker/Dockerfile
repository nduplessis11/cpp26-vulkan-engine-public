# syntax=docker/dockerfile:1.7

# Native Linux build environment. Wayland dev packages match the baseline
# docs/BUILDING.md documents for a host Linux build; no GLFW or X11 packages,
# since ADR 0006 has no GLFW dependency and XCB is not implemented yet.
FROM gcc:16.1.0 AS linux

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
        ca-certificates \
        ccache \
        cmake \
        git \
        libvulkan-dev \
        libwayland-dev \
        libxkbcommon-dev \
        ninja-build \
        pkg-config \
        vulkan-validationlayers \
        wayland-protocols \
    && rm -rf /var/lib/apt/lists/* \
    && mkdir -p /workspace /ccache \
    && chmod 0777 /workspace /ccache

ENV CCACHE_DIR=/ccache \
    CCACHE_MAXSIZE=20G \
    HOME=/tmp
WORKDIR /workspace

# Windows cross-compile environment: an x86_64-w64-mingw32 GCC toolchain
# running on Linux, the same base pattern Nathan Baggs publishes. The engine
# has no Windows Vulkan SDK import library here, so the docker-windows-mingw-
# debug preset builds scaffold-only (ENGINE_REQUIRE_VULKAN_SDK=OFF) until a
# lesson decides the Vulkan loading strategy for cross builds.
FROM ghcr.io/nathan-baggs/custom-mingw64:latest AS windows-mingw

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
        ccache \
    && rm -rf /var/lib/apt/lists/* \
    && mkdir -p /workspace /ccache \
    && chmod 0777 /workspace /ccache

ENV CCACHE_DIR=/ccache \
    CCACHE_MAXSIZE=20G \
    HOME=/tmp
WORKDIR /workspace
