.PHONY: images mingw-toolchain-image \
        configure-docker-linux build-docker-linux test-docker-linux \
        configure-docker-linux-asan build-docker-linux-asan test-docker-linux-asan \
        configure-docker-windows build-docker-windows \
        build-scratchpad run-scratchpad \
        ccache-stats clean

# Under Windows Git Bash/MSYS, `id -u` reports a Windows-mapped UID that Docker
# Desktop's bind mount does not recognize, and using it fails container writes with
# a permission-denied error. compose.yaml's own 1000:1000 default already matches
# what Docker Desktop's Windows file sharing expects, so skip id -u there entirely.
UNAME_S := $(shell uname -s 2>/dev/null)
ifneq (,$(findstring MINGW,$(UNAME_S))$(findstring MSYS,$(UNAME_S)))
LOCAL_UID := 1000
LOCAL_GID := 1000
else
LOCAL_UID := $(shell id -u 2>/dev/null || echo 1000)
LOCAL_GID := $(shell id -g 2>/dev/null || echo 1000)
endif
COMPOSE := LOCAL_UID=$(LOCAL_UID) LOCAL_GID=$(LOCAL_GID) docker compose

# docker/Dockerfile's windows-mingw stage builds FROM this tag. GCC 17 has
# no published image, so unlike the old GCC 16 setup this base must be built
# locally before compose can resolve it; see docs/BUILDING.md.
mingw-toolchain-image:
	docker build -f docker/Dockerfile.mingw-from-source -t cpp26-vulkan-engine-mingw:17-trunk .

images: mingw-toolchain-image
	$(COMPOSE) build linux windows-mingw

configure-docker-linux:
	$(COMPOSE) run --rm linux cmake --preset docker-linux-gcc-debug

build-docker-linux: configure-docker-linux
	$(COMPOSE) run --rm linux cmake --build --preset docker-linux-gcc-debug

test-docker-linux: build-docker-linux
	$(COMPOSE) run --rm linux ctest --preset docker-linux-gcc-debug

# Same three steps as the docker-linux targets, against a preset that turns on
# -fsanitize=address,undefined. It uses its own build directory, so switching
# between the two does not force a rebuild each way.
configure-docker-linux-asan:
	$(COMPOSE) run --rm linux cmake --preset docker-linux-gcc-asan

build-docker-linux-asan: configure-docker-linux-asan
	$(COMPOSE) run --rm linux cmake --build --preset docker-linux-gcc-asan

test-docker-linux-asan: build-docker-linux-asan
	$(COMPOSE) run --rm linux ctest --preset docker-linux-gcc-asan

# apps/scratchpad links engine::runtime, which is header-only (INTERFACE), so
# --target engine_scratchpad still builds two files instead of the whole graph. Run it in
# the container, not on the host: it links the image's from-source GCC 17 libstdc++,
# which is newer than a typical host's (GLIBCXX version error otherwise).
# Pass arguments through with ARGS, e.g. `make run-scratchpad ARGS="--verbose 3"`.
SCRATCHPAD_BIN := ./out/build/docker-linux-gcc-debug/apps/scratchpad/engine_scratchpad

build-scratchpad: configure-docker-linux
	$(COMPOSE) run --rm linux cmake --build --preset docker-linux-gcc-debug --target engine_scratchpad

run-scratchpad: build-scratchpad
	$(COMPOSE) run --rm linux $(SCRATCHPAD_BIN) $(ARGS)

configure-docker-windows:
	$(COMPOSE) run --rm windows-mingw cmake --preset docker-windows-mingw-debug

build-docker-windows: configure-docker-windows
	$(COMPOSE) run --rm windows-mingw cmake --build --preset docker-windows-mingw-debug

ccache-stats:
	$(COMPOSE) run --rm linux ccache --show-stats
	$(COMPOSE) run --rm windows-mingw ccache --show-stats

clean:
	rm -rf out
