.PHONY: images mingw-toolchain-image \
        configure-docker-linux build-docker-linux test-docker-linux \
        configure-docker-linux-asan build-docker-linux-asan test-docker-linux-asan \
        configure-docker-windows build-docker-windows \
        build-scratchpad run-scratchpad \
        build-sandbox run-sandbox \
        configure-clang-tidy lint lint-fix format-cmake check-cmake-format \
        ccache-stats clean

# Use 1000:1000 on Windows: Git Bash/MSYS ids cause Docker Desktop write errors.
# On Linux, use the current user's ids so container-created files have the right owner.
UNAME_S := $(shell uname -s 2>/dev/null)
ifneq (,$(findstring MINGW,$(UNAME_S))$(findstring MSYS,$(UNAME_S)))
LOCAL_UID := 1000
LOCAL_GID := 1000
else
LOCAL_UID := $(shell id -u 2>/dev/null || echo 1000)
LOCAL_GID := $(shell id -g 2>/dev/null || echo 1000)
endif
COMPOSE := LOCAL_UID=$(LOCAL_UID) LOCAL_GID=$(LOCAL_GID) docker compose

# Build the local base image required by the windows-mingw stage.
# See docs/BUILDING.md.
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

# Enable AddressSanitizer and UndefinedBehaviorSanitizer in a separate build
# directory so switching presets does not rebuild the other configuration.
configure-docker-linux-asan:
	$(COMPOSE) run --rm linux cmake --preset docker-linux-gcc-asan

build-docker-linux-asan: configure-docker-linux-asan
	$(COMPOSE) run --rm linux cmake --build --preset docker-linux-gcc-asan

test-docker-linux-asan: build-docker-linux-asan
	$(COMPOSE) run --rm linux ctest --preset docker-linux-gcc-asan

# Build only the scratchpad target. Run it in the container so it uses the matching
# GCC 17 libstdc++; an older host library can cause GLIBCXX version errors.
# Pass arguments with ARGS, e.g. `make run-scratchpad ARGS="--verbose 3"`.
SCRATCHPAD_BIN := ./out/build/docker-linux-gcc-debug/apps/scratchpad/engine_scratchpad

build-scratchpad: configure-docker-linux
	$(COMPOSE) run --rm linux cmake --build --preset docker-linux-gcc-debug --target engine_scratchpad

run-scratchpad: build-scratchpad
	$(COMPOSE) run --rm linux $(SCRATCHPAD_BIN) $(ARGS)

# Run in the container to use the same GCC 17 libstdc++ as the build.
SANDBOX_BIN := ./out/build/docker-linux-gcc-debug/apps/sandbox/engine_sandbox

build-sandbox: configure-docker-linux
	$(COMPOSE) run --rm linux cmake --build --preset docker-linux-gcc-debug --target engine_sandbox

run-sandbox: build-sandbox
	$(COMPOSE) run --rm linux $(SANDBOX_BIN)

configure-docker-windows:
	$(COMPOSE) run --rm windows-mingw cmake --preset docker-windows-mingw-debug

build-docker-windows: configure-docker-windows
	$(COMPOSE) run --rm windows-mingw cmake --build --preset docker-windows-mingw-debug

# Use host tools for clang-tidy; the image has no Clang and uses newer GCC headers.
# This preset creates compile_commands.json without the unsupported -fcontracts flag.
configure-clang-tidy:
	cmake --preset linux-clang-tidy

# run-clang-tidy checks every matching file, even when no rebuild is needed.
# Limit it to this repo's sources; the database also contains GoogleTest files.
LINT_SOURCES := ^$(CURDIR)/(engine|apps|tests)/

# Clang has not implemented P2900 (clang.llvm.org/cxx_status.html lists Contracts
# as "No"), so it cannot parse pre, post, or contract_assert. Erase them before
# clang-tidy sees them, the same way .clangd does for the editor. GCC compiles and
# enforces the real clauses. Cost: any call named pre or post is erased too, so
# clang-tidy would analyse code GCC never compiles.
CONTRACT_STUBS := '-extra-arg=-Dpre(...)=' '-extra-arg=-Dpost(...)=' '-extra-arg=-Dcontract_assert(...)=((void)0)'

# clang-tidy has no line-length check, so lint also runs clang-format, which
# enforces ColumnLimit and the rest of .clang-format. It skips
# `clang-format off` regions.
FORMAT_SOURCES = $(shell find engine apps tests -name '*.[ch]pp')

# Limit formatting to source directories so generated and fetched CMake files
# under out/ and cmake-build-*/ are excluded.
CMAKE_FORMAT_SOURCES = CMakeLists.txt $(shell find cmake engine apps tests -name CMakeLists.txt -o -name '*.cmake')

format-cmake:
	printf '%s\n' $(CMAKE_FORMAT_SOURCES) | grep -E '$(or $(FILE),.)' | xargs -r gersemi --no-cache --in-place

check-cmake-format:
	gersemi --no-cache --check $(CMAKE_FORMAT_SOURCES)

lint: check-cmake-format configure-clang-tidy
	clang-format --dry-run --Werror $(FORMAT_SOURCES)
	run-clang-tidy -p out/build/linux-clang-tidy -quiet $(CONTRACT_STUBS) "$(LINT_SOURCES)"

# Apply clang-tidy fixes in place, then run clang-format, which wraps long lines.
# FILE limits each pass to sources matching that regex, e.g.
# `make lint-fix FILE=tests/platform/window_test.cpp`. Headers are not in the
# database; their clang-tidy fixes come from the .cpp files that include them.
# Also format CMake files. Run every pass and return the first failure.
lint-fix: configure-clang-tidy
	@tidy_status=0; \
	run-clang-tidy -p out/build/linux-clang-tidy -quiet -fix $(CONTRACT_STUBS) "$(or $(FILE),$(LINT_SOURCES))" || tidy_status=$$?; \
	format_status=0; \
	printf '%s\n' $(FORMAT_SOURCES) | grep -E '$(or $(FILE),.)' | xargs -r clang-format -i || format_status=$$?; \
	cmake_format_status=0; \
	$(MAKE) --no-print-directory format-cmake || cmake_format_status=$$?; \
	if [ $$tidy_status -ne 0 ]; then \
	    exit $$tidy_status; \
	fi; \
	if [ $$format_status -ne 0 ]; then \
	    exit $$format_status; \
	fi; \
	exit $$cmake_format_status

ccache-stats:
	$(COMPOSE) run --rm linux ccache --show-stats
	$(COMPOSE) run --rm windows-mingw ccache --show-stats

clean:
	rm -rf out
