# Debugging playbook

1. Reproduce the smallest deterministic case.
2. Record the command, backend, GPU and driver, validation state, and last good commit.
3. Read the first meaningful error rather than its cascade.
4. State one hypothesis and the observation that would falsify it.
5. Inspect ownership, parent lifetime, resource state, queue, and completion token.
6. Add a temporary invariant, counter, or debug name rather than sleeps or global waits.
7. Compare CPU intent with a GPU capture when the command stream is suspect.
8. Add regression proof.
9. Record the learned invariant in `state/LEARNING_LOG.md`.

`vkDeviceWaitIdle` may be a temporary labeled diagnostic, not the final lifetime strategy.

## GDB / VS Code (Linux)

The everyday Linux debug loop builds natively (not through Docker), so there is no
source-path remapping to configure: the compiler and GDB both see the same absolute
path.

```bash
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug
```

The debug binary lands at `out/build/linux-gcc-debug/apps/sandbox/engine_sandbox`.

**VS Code:** copy `.vscode/launch.example.json` to `.vscode/launch.json` and
`.vscode/tasks.example.json` to `.vscode/tasks.json` (both stay gitignored — they're
your personal copies), then set a breakpoint in a project `.cpp` file and press F5.
The launch config's `preLaunchTask` runs an incremental `cmake --build`, not a full
reconfigure, so it will not trigger a GCC rebuild.

**Plain GDB**, for the same binary:

```bash
gdb out/build/linux-gcc-debug/apps/sandbox/engine_sandbox
(gdb) break apps/sandbox/main.cpp:11
(gdb) run
(gdb) bt
(gdb) info locals
```

**Debugging a Docker-built binary directly on the host instead:** `compose.yaml` mounts
the repository into the `linux` container at `/workspace`, so a binary built via the
`docker-linux-gcc-debug` preset has `/workspace/...` baked into its DWARF info, not your
host's real repository path. GDB will fail to find source until you add a path
substitution:

```gdb
(gdb) set substitute-path /workspace /absolute/path/to/cpp26-vulkan-engine
```

This is the same class of problem the Windows cross-build hits below — it just doesn't
affect the native `linux-gcc-debug` preset because nothing is mounted or path-remapped
there at all.

**Pretty-printers not loading?** A GCC installed under a nonstandard prefix (e.g. a
self-built `/usr/local`, as both Docker images use for their pinned GCC 17 trunk) ships
its libstdc++ pretty-printer script next to the library itself, outside GDB's default
`auto-load safe-path`. GDB will silently decline to load it and print a warning instead
of showing readable `std::string`/`std::vector`/etc. Fix it once per machine:

```gdb
echo "add-auto-load-safe-path /usr/local/lib64" >> ~/.config/gdb/gdbinit
```

(adjust the path to wherever your GCC's `libstdc++.so.*-gdb.py` actually lives).

## GDB / VS Code (Windows)

The canonical Windows path builds in Docker, then runs and debugs the resulting `.exe`
directly on Windows — Docker never runs the graphical application.

```powershell
docker compose run --rm windows-mingw cmake --preset docker-windows-mingw-debug
docker compose run --rm windows-mingw cmake --build --preset docker-windows-mingw-debug
```

(or `make configure-docker-windows` / `make build-docker-windows`). The `.exe` lands at
`out/build/docker-windows-mingw-debug/apps/sandbox/engine_sandbox.exe`. It statically
links its MinGW runtime (`cmake/WindowsRuntime.cmake`) — verified by inspecting its
imports (`x86_64-w64-mingw32-objdump -p`), which show only `KERNEL32.dll` and the
Universal CRT's `api-ms-win-crt-*.dll` set (shipped by Windows itself), no
`libstdc++-6.dll`/`libgcc_s_seh-1.dll`/`libwinpthread-1.dll`. It runs directly — nothing
to stage.

**Install a MinGW-w64 GDB on Windows.** This has to be a native Windows program that
understands PE binaries — the container's `gdb` cannot run on Windows at all. Any
x86_64 MinGW-w64 GDB build works (an MSYS2 `mingw-w64-x86_64-gdb` install, or a
standalone MinGW-w64 distribution). Point `launch.json` at it with an environment
variable instead of a hardcoded path, set once:

```powershell
setx ENGINE_MINGW_GDB "C:\mingw64\bin\gdb.exe"
```

(open a new terminal / restart VS Code afterward so the variable takes effect).

**VS Code:** copy `.vscode/launch.example.json` to `.vscode/launch.json` and
`.vscode/tasks.example.json` to `.vscode/tasks.json`, then pick "Engine — Windows MinGW
Debug" and press F5.

**Source-path mapping.** Compose mounts the repo into the `windows-mingw` container at
`/workspace`, so the `.exe`'s DWARF records `/workspace/apps/sandbox/main.cpp`, not your
real clone path — same class of problem as debugging a Docker-built Linux binary
directly on the host (see above), just unconditional here since there is no native
Windows cross-compiler to sidestep it with. The launch config's `sourceFileMap`
(`{"/workspace": "${workspaceFolder}"}`) handles this for VS Code automatically. From
plain GDB:

```gdb
(gdb) set substitute-path /workspace C:/dev/cpp26-vulkan-engine
(gdb) break apps/sandbox/main.cpp:11
(gdb) run
```

Verified for real: a native MinGW-w64 GDB (GDB 16.3) hit the breakpoint (reported at
line 12 — GDB lands on the first line with its own instruction, one past line 11's
declaration-only statement, same as the Linux proof), showed a correct backtrace with
`No locals` at that point, then resumed and produced the expected scaffold output — with
no container involved at any point after the build.

## GDB / Zed

Zed (1.13+) ships a built-in debugger that talks to GDB directly — no separate
extension needed. Configurations live in `.zed/debug.json`, gitignored the same way
`.vscode/launch.json` is; copy `.zed/debug.example.json` to `.zed/debug.json` for your
personal copy, then open the Debug panel, pick "Engine — Linux GCC Debug" or "Engine —
Windows MinGW Debug", set a breakpoint, and start the session.

Each configuration's `build` step runs the same incremental build command as its VS
Code counterpart (see above), not a full reconfigure — run the configure step from the
command line once first.

**Windows: `gdb_path` needs a literal, host-specific path.** Unlike VS Code's
`${env:ENGINE_MINGW_GDB}`, Zed does not expand OS environment variables inside
`debug.json` (an open upstream limitation as of 1.13). Edit `gdb_path` in your local
copy directly instead of relying on the `ENGINE_MINGW_GDB` env var from the VS Code
section above. The example ships with the same path verified there
(`C:/mingw64/bin/gdb.exe`, GDB 16.3) — edit it if yours lives elsewhere.

**Windows: source-path remap.** Same `/workspace`-vs.-real-repo-path problem as the VS
Code entry (see above) — there's no dedicated `sourceFileMap`-style field, so the
example does the remap the way plain GDB does, via `gdb_args`: `-ex "set
substitute-path /workspace $ZED_WORKTREE_ROOT"`.

**Unverified end-to-end.** `.zed/debug.example.json` was checked against Zed's
documented `debug.json` schema and is valid JSON, but this environment can't drive
Zed's GUI debug panel, so no session was actually launched from here the way the
Windows VS Code config above was. If breakpoints don't bind, libstdc++ pretty-printing
doesn't kick in, or `gdb_args` behaves differently than expected, that's the first
place to look — Zed's GDB adapter is newer and thinner than VS Code's cpptools
integration, with open upstream issues around environment/argument handling.
