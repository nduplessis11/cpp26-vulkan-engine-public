# Dependency policy

## Baseline infrastructure

Allowed without a special architecture vote:

- C++ compiler and standard library;
- CMake, Ninja, and repository Python scripts;
- operating-system APIs and development headers;
- Vulkan loader, headers, validation, and SDK tools;
- shader compiler and tooling;
- Wayland client libraries, wayland-protocols, wayland-scanner, and xkbcommon;
- XCB and Win32 system APIs.

## Engine dependencies

A new library requires an ADR answering the exact problem, whether it hides the current lesson, runtime versus tool scope, pinning, update and license policy, and removal fallback.

## Default learning choices

- Own math rather than GLM.
- Own GPU allocator exercises rather than VMA.
- Own sparse-set ECS rather than EnTT.
- Own a small UI and editor path rather than making ImGui the architecture.
- Small TGA, OBJ, and WAV parsers before production format libraries.
- No SDL or GLFW in the native-platform path.
- No generic renderer abstraction library.

This policy may change through evidence and ADRs; it is not ideology.
