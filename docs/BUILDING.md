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
cmake --preset linux-clang-debug
cmake --build --preset linux-clang-debug
ctest --preset linux-clang-debug
```

Use `linux-gcc-debug` for GCC cross-checks.

## Windows

Use a current Visual Studio Build Tools Developer PowerShell, then:

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug
```

## Scaffold-only inspection

```bash
cmake --preset scaffold-no-vulkan
cmake --build --preset scaffold-no-vulkan
ctest --preset scaffold-no-vulkan
```

This does not satisfy lesson 000.
