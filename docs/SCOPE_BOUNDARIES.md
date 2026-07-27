# Scope boundaries

## Supported core target

- 64-bit desktop Windows and Linux.
- Vulkan 1.4-capable hardware and drivers.
- Native Win32, Wayland, and later XCB/X11 compatibility.
- One Vulkan renderer; no Direct3D, OpenGL, or Metal abstraction.
- 2D and 3D in one resource and frame architecture.
- Development editor and asset tools.

## Deliberately outside the core roadmap

- Consoles, mobile, web, and macOS portability.
- Networking and multiplayer.
- AAA-scale world streaming.
- Full production rigid-body middleware parity.
- Full Unicode shaping and font stack.
- Reimplementing complex PNG, JPEG, audio compression, and full glTF specifications before fundamentals.
- A stable third-party plugin ABI before the engine stabilizes.

## “Write everything myself”

You write platform integration, Vulkan renderer, wrappers, allocators, math, ECS, scene and assets, 2D and 3D systems, editor UI, basic physics and audio, tests, and tools.

You use the compiler, standard library, CMake and Ninja, OS APIs, Vulkan loader and headers, validation, shader compiler, and Wayland protocol definitions and scanner as infrastructure.

Later production-grade format libraries require an ADR. The curriculum first includes small TGA, OBJ, and WAV implementations.
