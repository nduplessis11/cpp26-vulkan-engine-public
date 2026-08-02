.PHONY: images \
        configure-docker-linux build-docker-linux test-docker-linux \
        configure-docker-windows build-docker-windows \
        ccache-stats clean

LOCAL_UID := $(shell id -u 2>/dev/null || echo 1000)
LOCAL_GID := $(shell id -g 2>/dev/null || echo 1000)
COMPOSE := LOCAL_UID=$(LOCAL_UID) LOCAL_GID=$(LOCAL_GID) docker compose

images:
	$(COMPOSE) build linux windows-mingw

configure-docker-linux:
	$(COMPOSE) run --rm linux cmake --preset docker-linux-gcc-debug

build-docker-linux: configure-docker-linux
	$(COMPOSE) run --rm linux cmake --build --preset docker-linux-gcc-debug

test-docker-linux: build-docker-linux
	$(COMPOSE) run --rm linux ctest --preset docker-linux-gcc-debug

configure-docker-windows:
	$(COMPOSE) run --rm windows-mingw cmake --preset docker-windows-mingw-debug

build-docker-windows: configure-docker-windows
	$(COMPOSE) run --rm windows-mingw cmake --build --preset docker-windows-mingw-debug

ccache-stats:
	$(COMPOSE) run --rm linux ccache --show-stats
	$(COMPOSE) run --rm windows-mingw ccache --show-stats

clean:
	rm -rf out
