# Building

## Common requirements

- CMake 3.25 or newer.
- Ninja or another supported generator.
- A compiler with usable C++26 or latest mode.
- Python 3.
- Vulkan 1.4 headers, loader, validation, and a Vulkan 1.4-capable driver.
- A shader compiler becomes required in lesson 030.

Run `python tools/check_environment.py`.

## Linux

The core track starts with Wayland. Development packages eventually include Wayland client, wayland-protocols, xkbcommon, pkg-config, and later XCB. Package names vary by distribution.

```bash
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug
```

## Windows

Use a current MSYS2 MinGW shell, then:

```powershell
cmake --preset windows-gcc-debug
cmake --build --preset windows-gcc-debug
ctest --preset windows-gcc-debug
```

## Scaffold-only inspection

```bash
cmake --preset scaffold-no-vulkan
cmake --build --preset scaffold-no-vulkan
ctest --preset scaffold-no-vulkan
```

This does not satisfy lesson 000.

## Docker (containerized builds)

Two Linux containers give a reproducible build environment without installing
a toolchain on the host: `linux` builds a native Linux binary, and
`windows-mingw` cross-compiles a native Windows `.exe` using an
`x86_64-w64-mingw32` GCC toolchain. Neither container runs the graphical
application; run the resulting Linux binary on Linux and the `.exe` on
Windows.

Both containers currently build GCC 17 from a dated trunk snapshot, not a
numbered release: GCC 17 has not shipped yet (see `docker/Dockerfile`'s
`linux-gcc-builder` stage and `docker/Dockerfile.mingw-from-source`'s
`GCC_SNAPSHOT` build arg). This is unreleased, experimental compiler
behavior -- expect it to need re-pinning to a newer snapshot occasionally,
and expect the occasional trunk regression. Revert both to a numbered GCC
release once one exists.

`windows-mingw` has no published image to fall back to for GCC 17 (the
`ghcr.io/nathan-baggs/custom-mingw64` image tracks a released GCC), so its
cross toolchain must be built locally first:

```bash
docker build -f docker/Dockerfile.mingw-from-source -t cpp26-vulkan-engine-mingw:17-trunk .

docker compose build linux windows-mingw

docker compose run --rm linux cmake --preset docker-linux-gcc-debug
docker compose run --rm linux cmake --build --preset docker-linux-gcc-debug
docker compose run --rm linux ctest --preset docker-linux-gcc-debug

docker compose run --rm windows-mingw cmake --preset docker-windows-mingw-debug
docker compose run --rm windows-mingw cmake --build --preset docker-windows-mingw-debug
```

The equivalent `make images`, `make build-docker-linux`,
`make test-docker-linux`, and `make build-docker-windows` targets wrap the
same commands; `make images` already builds the mingw toolchain image first.
Both the native GCC 17 bootstrap (`linux-gcc-builder` stage) and the cross
toolchain bootstrap take noticeably longer than pulling a prebuilt image did
-- this is a one-time cost per Docker layer cache, not a per-build cost.

The `docker-windows-mingw-debug` preset builds scaffold-only
(`ENGINE_REQUIRE_VULKAN_SDK=OFF`): the Linux container has no Windows Vulkan
SDK import library, so this preset does not link Vulkan yet. It exists to
prove the cross-compiler and CMake wiring; a later lesson decides how the
engine loads Vulkan for cross-compiled Windows builds. `docker-linux-gcc-debug`
installs a real Vulkan loader and links normally.

On Linux hosts, set `LOCAL_UID`/`LOCAL_GID` (see `.env.example`) if your user
is not UID/GID 1000, so files written by the container remain owned by you.

### Reverting the Windows cross toolchain to a published image

`docker/Dockerfile`'s `windows-mingw` stage currently builds FROM the local
`cpp26-vulkan-engine-mingw:17-trunk` image (see `docker/Dockerfile.mingw-from-source`),
because the published `ghcr.io/nathan-baggs/custom-mingw64` image tracks a
released GCC and GCC 17 has not released. That local image must exist before
`docker compose build windows-mingw` can resolve the `FROM`; `make images`
and the explicit `docker build -f docker/Dockerfile.mingw-from-source ...`
command above both produce it.

To go back to the published image instead -- e.g. to sidestep trunk risk,
accepting whatever GCC 16.x that image currently ships -- change
`docker/Dockerfile`'s `windows-mingw` stage back to:

```dockerfile
FROM ghcr.io/nathan-baggs/custom-mingw64:latest AS windows-mingw
```

and rebuild the compose image (`docker compose build windows-mingw`). Once
GCC 17 actually ships a numbered release, re-pin `GCC_SNAPSHOT` in both
Dockerfiles to that release and drop "trunk" from the image tags. Separately,
`Dockerfile.mingw-from-source`'s `MINGW_REF` tracks mingw-w64 master only
because its `__stack_chk_guard` fix for GCC 17 hasn't reached a release
either (see that file's header comment); re-pin `MINGW_REF` to a numbered
mingw-w64 release once one includes the fix.
