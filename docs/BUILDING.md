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

The canonical Windows path cross-compiles in Docker on Linux, then runs and debugs the
resulting `.exe` directly on Windows -- see "Docker (containerized builds)" below for the
exact commands. Docker is the build environment here, not where you run or debug.

The produced `.exe` statically links its MinGW runtime (`-static -static-libgcc
-static-libstdc++`, via `cmake/WindowsRuntime.cmake`), so it depends only on core Windows
DLLs and there is nothing to stage next to it.

### Alternative: native MSYS2 build

If you already have a working MSYS2 MinGW GCC 17 environment and prefer to build directly
on Windows without Docker:

```powershell
cmake --preset windows-gcc-debug
cmake --build --preset windows-gcc-debug
ctest --preset windows-gcc-debug
```

This preset is a fallback, not the primary path: keeping a GCC 17 trunk snapshot working
through MSYS2 is on you to maintain, versus the Docker path's pinned, reproducible
toolchain.

## Scaffold-only inspection

```bash
cmake --preset scaffold-no-vulkan
cmake --build --preset scaffold-no-vulkan
ctest --preset scaffold-no-vulkan
```

This does not satisfy lesson 000.

## Debug build diagnostics

Every project-owned target (via `engine::project_options`) picks up two configure-time
safeguards, added by `cmake/DebugDiagnostics.cmake` and `cmake/CompilerChecks.cmake`:

- **Debug flags.** CMake's own `Debug` config only guarantees a bare `-g` for GCC. Debug
  configures now also apply `-Og -g3 -fno-omit-frame-pointer` (GCC/MinGW only) so stepping,
  local-variable inspection, and backtraces stay reliable. Look for these flags in a
  verbose build (`cmake --build --preset linux-gcc-debug -- -v`) to confirm they landed.
- **Compiler identity check.** Configure fails immediately with `FATAL_ERROR` if
  `CMAKE_CXX_COMPILER_ID` isn't `GNU`, or if a Windows cross-compile preset resolves a
  compiler that isn't the `x86_64-w64-mingw32` toolchain. The configure log also prints
  the resolved compiler identity and version (`Compiler identity check passed: ...`) so
  you can confirm which GCC you actually built with.
- **Language contracts.** `ENGINE_ENABLE_CONTRACTS` defaults to `ON`.
  `cmake/Contracts.cmake` checks `__cpp_contracts` and compiles and links a sample
  containing `pre`, `post`, and `contract_assert`. Configure fails if the probe
  fails. Every build compiles and links with `-fcontracts`; Debug selects
  `-fcontract-evaluation-semantic=enforce`, and other configurations select
  `ignore`. `NDEBUG` does not control these checks. `ENGINE_ENABLE_CONTRACTS=OFF`
  is reserved for the configure-only Clang tooling preset, whose binaries are
  not supported. See [ADR 0010](adr/0010-prefer-language-contracts.md).

The pinned MinGW library needs a forwarding definition for its default contract
handler. CMake adds `cmake/compat/mingw_contract_handler.cpp` only if the normal
link probe fails and a probe with that definition passes. Contract diagnostics
still come from GCC's default handler.

## Editor contracts (clangd)

clangd 22.1.8 rejects `pre`, `post`, and `contract_assert`, even after removing
`-fcontracts`. The root `.clangd` removes GCC's contracts flags and defines
macros that discard contract clauses during clangd preprocessing. `pre` and
`post` expand to nothing; `contract_assert` expands to `((void)0)` so it remains
a statement when followed by a semicolon.

These definitions apply only to clangd. GCC still compiles the original contracts.
Keep the definitions out of CMake and source headers. clangd cannot diagnose
contract expressions or find references inside them. The macros also affect
unrelated functions named `pre` or `post`, including member calls.

The compilation database path remains `out/build/linux-gcc-debug`. Update
`CompilationDatabase` in `.clangd` when switching presets. Restart the editor's
clangd instance if it retains old diagnostics. Check a file with:

```bash
clangd --check=apps/scratchpad/main.cpp --enable-config --log=verbose
```

The log shows the adjusted compile command. See clangd's
[CompileFlags documentation](https://clangd.llvm.org/config#compileflags).

`make lint` does not read `.clangd`, so the `Makefile` passes the same three
definitions to `run-clang-tidy` as `-extra-arg`. Keep the two lists in step: a
macro added in one place and not the other means the editor and the lint pass
disagree about what the code says.

## CMake formatting

Install [Gersemi](https://github.com/BlankSpruce/gersemi) on the host:

```bash
pipx install 'gersemi==0.28.1'
```

The repository's `.gersemirc` sets four-space indentation and an 80-column
limit. It also enables Gersemi's checks that formatting preserves the parsed
code. Format or check CMake files without configuring or building the engine:

```bash
make format-cmake
make format-cmake FILE=cmake/WaylandProtocols.cmake
make check-cmake-format
```

These commands cover the root `CMakeLists.txt` and CMake files under `cmake/`,
`engine/`, `apps/`, and `tests/`. They exclude generated build files.
`make lint` checks CMake formatting, and `make lint-fix` applies it.

For formatting on save, configure the editor to run Gersemi with the repository's
`.gersemirc`. Editors that send the buffer through standard input can use
`gersemi --no-cache --config /path/to/repo/.gersemirc -`.

## Static analysis (clang-tidy)

```bash
make lint
```

This configures the `linux-clang-tidy` preset and then runs `run-clang-tidy` over the
resulting `compile_commands.json`. A full pass takes about six seconds and needs no
build.

Before clang-tidy runs, `make lint` runs `clang-format --dry-run --Werror` over
`engine/`, `apps/`, and `tests/` and stops on any file that differs from
`.clang-format`, including lines longer than `ColumnLimit` (80). clang-tidy has no
line-length check.

`make lint` only reports. To apply the suggested fixes in place:

```bash
make lint-fix                                     # every lint source
make lint-fix FILE=tests/platform/window_test.cpp # one file
```

`lint-fix` runs clang-tidy's fixes, `clang-format -i`, and Gersemi. Formatting
still runs when clang-tidy fails. The target returns the first failing pass's
exit status.
It cannot wrap lines inside `// clang-format off` regions; wrap those by hand.

`FILE` is a regex matched against paths in `compile_commands.json` and against
relative source paths for clang-format and Gersemi. Headers are not in
`compile_commands.json`, so pass a `.cpp` that includes the header; clang-tidy
fixes inside the header are applied once even when several files include it.

Four things about this setup are deliberate:

- **`run-clang-tidy`, not `CMAKE_CXX_CLANG_TIDY`.** The `ENGINE_ENABLE_CLANG_TIDY`
  option drives clang-tidy as part of compilation, which means an up-to-date build
  tree reports `ninja: no work to do` and analyzes nothing. `run-clang-tidy` reads the
  compilation database directly, so every invocation is a complete pass. Keep the
  option for editor integration; use `make lint` to actually check the tree.
- **Host, not Docker.** `docker/Dockerfile` installs no clang, and the image's
  from-source GCC 17 libstdc++ is newer than any released clang can parse.
- **Contracts off.** clang rejects GCC's `-fcontracts`, and every translation unit
  carrying it fails to parse, so the preset sets `ENGINE_ENABLE_CONTRACTS=OFF`. The
  preset is a configure step only; its binaries are not the ones to test against.
- **Only files this repository owns.** `run-clang-tidy` takes a path regex, and the
  Makefile passes one that keeps the pass inside `engine/`, `apps/`, and `tests/`.
  Fetched googletest sources appear in `compile_commands.json` too, and `.clang-tidy`
  cannot filter them out: `HeaderFilterRegex` filters *headers*, while a translation
  unit's own source file is always analyzed.

`WarningsAsErrors` is empty in `.clang-tidy`, so findings never fail anything. No
build or test preset enables clang-tidy, so this is an on-demand pass rather than a
cost every build pays.

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
`make test-docker-linux`, `make build-docker-windows`, and
`make run-scratchpad` targets wrap the same commands; `make images` already
builds the mingw toolchain image first.
Both the native GCC 17 bootstrap (`linux-gcc-builder` stage) and the cross
toolchain bootstrap take noticeably longer than pulling a prebuilt image did
-- this is a one-time cost per Docker layer cache, not a per-build cost.

### Scratchpad (throwaway experiments)

`apps/scratchpad` is a personal syntax/idea sandbox. It links
`engine::project_options`, `engine::project_warnings`, and `engine::runtime`.
Because `engine::runtime` is header-only (`INTERFACE`), that last one only adds
`engine/include` to the include path -- it compiles nothing, so the scratchpad
can exercise real engine headers while still building in two compile steps:

```bash
make run-scratchpad
make run-scratchpad ARGS="--verbose 3"   # pass arguments to the binary
```

That configures if needed, builds `--target engine_scratchpad` alone (two
compile steps, not the whole tree), and runs the binary in the `linux`
container. `make build-scratchpad` stops after the build. The raw form is:

```bash
docker compose run --rm linux cmake --build --preset docker-linux-gcc-debug --target engine_scratchpad
docker compose run --rm linux ./out/build/docker-linux-gcc-debug/apps/scratchpad/engine_scratchpad
```

The dependency runs one way only: scratchpad -> engine. Nothing under `engine/`
or `tests/` may depend on `engine_scratchpad`, and a lesson's acceptance
evidence belongs in `tests/`, never here.

Run it **inside** the container. The binary links the image's from-source GCC 17
libstdc++, which is typically newer than the host's, so running it on the host
fails with a `GLIBCXX_... not found` loader error.

Because `engine::project_options` carries `-fcontracts` in Debug (see
`cmake/Contracts.cmake`), `pre()`, `post()`, and `contract_assert()` are live
in the scratchpad -- useful for trying contract and `std::expected` shapes
before committing to them in engine code.

The `docker-windows-mingw-debug` preset builds scaffold-only
(`ENGINE_REQUIRE_VULKAN_SDK=OFF`): `Dockerfile.mingw-from-source` vendors
Vulkan-Headers into the cross sysroot (so `<vulkan/vulkan.h>` compiles), but there
is still no Windows Vulkan import library or `vulkan-1.dll`, so this preset does
not link Vulkan yet. It exists to prove the cross-compiler and CMake wiring; a
later lesson decides how the engine loads Vulkan for cross-compiled Windows
builds (most likely `LoadLibrary` + `vkGetInstanceProcAddr` rather than
link-time, since that needs no import library at all). `docker-linux-gcc-debug`
installs a real Vulkan loader and links normally.

On Linux hosts, set `LOCAL_UID`/`LOCAL_GID` (see `.env.example`) if your user
is not UID/GID 1000, so files written by the container remain owned by you. On
Windows hosts, leave them unset: `compose.yaml` defaults to `1000:1000`, which
is what Docker Desktop's bind mount expects, and the `Makefile` detects a
Windows shell (MSYS/MinGW `uname`) and skips its `id -u`-based override for the
same reason -- that override reports a Windows-mapped UID Docker Desktop does
not recognize, which fails with a permission-denied error on configure.

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
