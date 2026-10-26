# cpp26-vulkan-engine

A game engine written in C++26 on top of Vulkan 1.4. The goal is an engine whose
platform layer, renderer, asset pipeline, and editor are all written in this
repository instead of taken from large libraries. It is early: the repository
holds the build system, a logging and timing core, and the first part of the
Wayland platform layer. It does not render anything yet.

This repository is a read-only copy, published automatically from a private
development repository.

## Technical choices

These are fixed by the documents in [`docs/`](docs/) and the accepted records in
[`docs/adr/`](docs/adr/).

- **Language:** C++26 (required, or latest mode). New C++26 features are adopted
  one at a time after a feature test. Runtime invariants use language contracts
  (`pre`, `post`, `contract_assert`), and recoverable failures use
  `std::expected`.
- **Graphics:** Vulkan 1.4 is the minimum, checked at the loader and the physical
  device. Rendering uses dynamic rendering and synchronization2. Resources are
  bindless through descriptor indexing. There is no generic rendering
  abstraction layer: the renderer targets Vulkan only.
- **Platforms:** native Wayland with xdg-shell first, native Win32 on Windows,
  and X11 through XCB later as a compatibility backend. There is no SDL or GLFW
  dependency. A headless window backend exists for tests.
- **Compiler:** GCC. The build rejects other compilers at configure time. The
  Docker images build GCC 17 from a dated trunk snapshot, because GCC 17 has not
  been released.

## Current state

What exists and is tested:

- `engine/include/engine/core/`: a `Logger` with severity levels, categories,
  and `StderrSink` and `CaptureSink` output; a `FrameClock` that turns elapsed
  time into fixed update steps; a `Version` parser; a move-only `FakeHandle`
  over a fake driver handle.
- `engine/include/engine/platform/`: the `IWindow` interface, a `HeadlessWindow`
  implementation, and a `NativeWindowHandle` variant for Wayland, Win32, and XCB.
- `engine/src/platform/linux/wayland/`: a Wayland connection that binds
  `wl_compositor` and `xdg_wm_base` and reports their versions. It does not
  create a window yet. The xdg-shell protocol code is generated at build time
  from `third_party/wayland-protocols/`.
- `apps/sandbox`: a program that logs the toolchain and Vulkan header version,
  queries the Wayland globals, and runs 600 frames of a headless window through
  the frame clock. It does not create a Vulkan instance or draw.
- `tests/`: 79 GoogleTest cases (core, window, Wayland registry) plus a smoke
  test and a test that runs the sandbox.

Not built yet: any Vulkan rendering, a real Wayland window, the Win32 and XCB
backends, math, ECS, scene, assets, audio, physics, and UI. The `engine/src/`
folders for these hold a README only. So do `apps/editor`, `apps/asset_cooker`,
`shaders/`, `tools/shader`, `tools/asset_cooker`, and `packaging/`.

## Building and testing

You need CMake 3.25 or newer, Ninja, Python 3, and a GCC with C++26 and
`-fcontracts` support (the Docker images below provide one). Native Linux builds
also need Wayland client development packages, `wayland-scanner`, `pkg-config`,
and the Vulkan 1.4 headers and loader. GoogleTest is fetched at configure time,
so the first configure needs network access. See
[`docs/BUILDING.md`](docs/BUILDING.md) for details.

**Docker (Linux build, no host toolchain):**

```bash
make test-docker-linux
make run-sandbox
```

`make test-docker-linux` configures, builds, and runs `ctest` with the
`docker-linux-gcc-debug` preset. The first run builds GCC 17 inside the image,
which takes a long time. `make run-sandbox` runs the sandbox in the container and
shares your Wayland socket if one exists. An AddressSanitizer and UBSan build is
available with `make test-docker-linux-asan`.

**Native Linux** (needs GCC 17 with contracts on your machine):

```bash
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug
```

Copy `CMakeUserPresets.json.example` to `CMakeUserPresets.json` to override the
compiler path. The `scaffold-no-vulkan` preset builds without the Vulkan SDK and
with the headless backend.

**Windows** is cross-compiled in Docker with an `x86_64-w64-mingw32` toolchain:

```bash
make images
make build-docker-windows
```

This build does not link Vulkan yet, and the container does not run the `.exe`.
Run it on Windows. A native MSYS2 build is also available with
`cmake --preset windows-gcc-debug`.

**Lint and formatting** (on the host): `make lint` runs clang-format, clang-tidy,
and a Gersemi check on the CMake files. `make lint-fix` applies the fixes.

## Layout

| Path | Contents |
| --- | --- |
| `engine/` | Engine headers in `include/engine/` and sources in `src/` |
| `apps/` | `sandbox` (built), `editor` and `asset_cooker` (planned) |
| `tests/` | GoogleTest and smoke tests, grouped by subsystem |
| `shaders/` | Shader source folders (planned) |
| `assets/` | Development assets (planned) |
| `cmake/` | Compiler checks, warnings, sanitizers, contracts, test helpers, toolchains |
| `third_party/` | Vendored `wayland-protocols` XML |
| `docker/`, `compose.yaml` | Linux and Windows cross build environments |
| `tools/`, `packaging/` | Shader and asset cooker tooling and packaging (planned) |
| `docs/` | Vision, architecture, platform strategy, build, testing, coding standards, ADRs |
