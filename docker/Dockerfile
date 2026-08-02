# syntax=docker/dockerfile:1.7

# GCC 17 has not cut a stable release yet -- it is trunk-only, dated
# snapshots -- so there is no published `gcc:17` image to pull the way
# `gcc:16.1.0` worked before. This stage bootstraps a native
# x86_64-linux-gnu GCC 17 trunk snapshot from source on the same Debian
# trixie base the old gcc:16.1.0 image used, so the apt package names below
# stay unchanged. Bump GCC_SNAPSHOT to track a newer trunk date; unlike a
# numbered release, this pin does not stop moving on its own and should be
# revisited once GCC 17 actually ships.
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

# No --disable-bootstrap: the default `make` target does a 3-stage build
# where stage 2 recompiles the compiler with itself and diffs the result
# against stage 3, so a trunk miscompilation fails this image build instead
# of silently shipping a broken compiler. The mingw cross toolchain built by
# docker/Dockerfile.mingw-from-source cannot get this check for free -- a
# cross compiler has no host to bootstrap against.
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

# Native Linux build environment. Wayland dev packages match the baseline
# docs/BUILDING.md documents for a host Linux build; no GLFW or X11 packages,
# since ADR 0006 has no GLFW dependency and XCB is not implemented yet.
FROM debian:trixie AS linux

COPY --from=linux-gcc-builder /usr/local /usr/local

# GCC's own runtime shared libraries (libstdc++, libgcc_s) install under
# /usr/local/lib64 on x86_64. Registering that with ldconfig is not enough:
# apt packages below pull in Debian trixie's own libstdc++6 (matching its
# system GCC 14), and ldconfig's cache does not reliably prefer a newly
# registered directory over the default system search path -- binaries
# linked against GCC 17's libstdc++ picked up the older system one instead
# and failed at runtime with a missing GLIBCXX symbol version. LD_LIBRARY_PATH
# is searched before the ld.so cache, so it deterministically wins.
RUN echo /usr/local/lib64 > /etc/ld.so.conf.d/gcc17-trunk.conf \
    && ldconfig

ENV PATH=/usr/local/bin:${PATH} \
    LD_LIBRARY_PATH=/usr/local/lib64

# The old gcc:16.1.0 base image came with a complete, ready-to-use toolchain
# (assembler, linker, glibc headers). A bare debian:trixie base plus our own
# /usr/local from linux-gcc-builder does not, so this stage has to supply
# what that image used to bring for free:
#   - binutils: as/ld/ar. cc1plus emits assembly; without `as` there is no
#     way to turn it into an object file.
#   - libc6-dev: glibc headers (features.h, stdio.h, ...) that any compile
#     needs; debian:trixie's base image only carries the runtime library.
#   - libgmp10/libisl23/libmpc3/libmpfr6/libzstd1: cc1plus itself is
#     dynamically linked against these (Graphite loop optimization, LTO/debug
#     info compression) because the linux-gcc-builder stage's -dev packages
#     were on the library search path at configure time. Their runtime-only
#     counterparts are enough to run the already-built compiler; without them
#     cc1plus fails to start.
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

# Windows cross-compile environment: an x86_64-w64-mingw32 GCC 17 trunk
# toolchain running on Linux. The published ghcr.io/nathan-baggs/custom-
# mingw64 image tracks a released GCC, so it cannot provide GCC 17 either --
# this stage always builds from docker/Dockerfile.mingw-from-source instead
# of falling back to a prebuilt image (see docs/BUILDING.md). That image vendors
# Vulkan-Headers into the target sysroot, so <vulkan/vulkan.h> compiles here, but
# there is still no Windows Vulkan import library or vulkan-1.dll -- the
# docker-windows-mingw-debug preset builds scaffold-only (ENGINE_REQUIRE_VULKAN_SDK=OFF)
# until a lesson decides the Vulkan linking/loading strategy for cross builds.
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
