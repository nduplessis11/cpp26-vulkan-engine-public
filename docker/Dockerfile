# syntax=docker/dockerfile:1.7

# Build native GCC from a dated GCC 17 snapshot on Debian trixie.
# Change GCC_SNAPSHOT to update the compiler; revisit the pin for a stable release.
FROM debian:trixie AS linux-gcc-builder

ARG GCC_SNAPSHOT=17-20260727

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
        bison \
        build-essential \
        ca-certificates \
        curl \
        flex \
        gperf \
        libgmp-dev \
        libisl-dev \
        libmpc-dev \
        libmpfr-dev \
        libzstd-dev \
        make \
        patch \
        texinfo \
        xz-utils \
        zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp/toolchain

# Keep GCC's three-stage bootstrap: comparing stages 2 and 3 checks for compiler
# miscompilation. The MinGW cross build cannot use this native bootstrap check.
RUN curl --fail --location --retry 3 \
        "https://gcc.gnu.org/pub/gcc/snapshots/${GCC_SNAPSHOT}/gcc-${GCC_SNAPSHOT}.tar.xz" \
        --output gcc.tar.xz \
    && tar -xf gcc.tar.xz \
    && mkdir build-gcc \
    && cd build-gcc \
    && ../gcc-${GCC_SNAPSHOT}/configure \
        --prefix=/usr/local \
        --enable-languages=c,c++ \
        --enable-threads=posix \
        --disable-multilib \
        --disable-nls \
        --disable-libstdcxx-pch \
        --with-system-zlib \
    && make -j"$(nproc)" \
    && make install \
    && cd / \
    && rm -rf /tmp/toolchain \
    && g++ --version \
    && printf '#include <version>\nint main(){}\n' \
        | g++ -std=c++26 -x c++ - -c -o /tmp/cpp26-test.o \
    && rm -f /tmp/cpp26-test.o

# Native Linux build environment with the Wayland dependencies in docs/BUILDING.md.
# ADR 0006 uses native backends; XCB dependencies will come with its implementation.
FROM debian:trixie AS linux

COPY --from=linux-gcc-builder /usr/local /usr/local

# Prefer GCC 17's runtime libraries in /usr/local/lib64 to Debian's older copies.
# ldconfig alone selected the older libstdc++ and caused GLIBCXX version errors;
# LD_LIBRARY_PATH takes priority over that cache.
RUN echo /usr/local/lib64 > /etc/ld.so.conf.d/gcc17-trunk.conf \
    && ldconfig

ENV PATH=/usr/local/bin:${PATH} \
    LD_LIBRARY_PATH=/usr/local/lib64

# The copied compiler still needs binutils (assembler/linker), libc6-dev (C headers),
# and its runtime libraries: libgmp10, libisl23, libmpc3, libmpfr6, and libzstd1.
RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
        binutils \
        ca-certificates \
        ccache \
        cmake \
        git \
        libc6-dev \
        libgmp10 \
        libisl23 \
        libmpc3 \
        libmpfr6 \
        libvulkan-dev \
        libwayland-dev \
        libxkbcommon-dev \
        libzstd1 \
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

# Build Windows binaries on Linux using the local GCC 17 MinGW image.
# Build that base with docker/Dockerfile.mingw-from-source; see docs/BUILDING.md.
# It includes Vulkan headers, but no Windows import library or vulkan-1.dll.
# The docker-windows-mingw-debug preset uses ENGINE_REQUIRE_VULKAN_SDK=OFF until
# a lesson defines how cross builds will link or load Vulkan.
FROM cpp26-vulkan-engine-mingw:17-trunk AS windows-mingw

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
