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

```bash
docker compose build linux windows-mingw

docker compose run --rm linux cmake --preset docker-linux-gcc-debug
docker compose run --rm linux cmake --build --preset docker-linux-gcc-debug
docker compose run --rm linux ctest --preset docker-linux-gcc-debug

docker compose run --rm windows-mingw cmake --preset docker-windows-mingw-debug
docker compose run --rm windows-mingw cmake --build --preset docker-windows-mingw-debug
```

The equivalent `make images`, `make build-docker-linux`,
`make test-docker-linux`, and `make build-docker-windows` targets wrap the
same commands.

The `docker-windows-mingw-debug` preset builds scaffold-only
(`ENGINE_REQUIRE_VULKAN_SDK=OFF`): the Linux container has no Windows Vulkan
SDK import library, so this preset does not link Vulkan yet. It exists to
prove the cross-compiler and CMake wiring; a later lesson decides how the
engine loads Vulkan for cross-compiled Windows builds. `docker-linux-gcc-debug`
installs a real Vulkan loader and links normally.

On Linux hosts, set `LOCAL_UID`/`LOCAL_GID` (see `.env.example`) if your user
is not UID/GID 1000, so files written by the container remain owned by you.

### Owning the Windows cross toolchain

`docker/Dockerfile`'s `windows-mingw` stage defaults to a published
`x86_64-w64-mingw32` GCC image, so its exact GCC version is whatever that
image currently ships (it will not always match the `linux` stage's pinned
GCC 16.1.0). To pin and fully own the cross toolchain instead, build the
from-source alternative:

```bash
docker build -f docker/Dockerfile.mingw-from-source -t cpp26-vulkan-engine-mingw:16.1 .
```

Then change `docker/Dockerfile`'s `windows-mingw` stage to:

```dockerfile
FROM cpp26-vulkan-engine-mingw:16.1 AS windows-mingw
```

and rebuild the compose image (`docker compose build windows-mingw`). This
takes noticeably longer to build the first time, since it compiles binutils,
MinGW-w64, and GCC from source rather than pulling a prebuilt layer.
