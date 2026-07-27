# C++26 policy

The repository requests C++26 with CMake and requires the requested mode. This does not mean every C++26 language and library feature is available or equally stable on every supported compiler.

1. Build in C++26 or latest mode.
2. Keep the engine baseline to features supported by active Clang, GCC, and MSVC targets.
3. Check feature-test macros and compiler status before adopting a C++26-only facility.
4. Introduce modules, contracts, reflection-related work, or major new library facilities through a small experiment and ADR.
5. Prefer clarity over novelty.
6. Do not let incomplete standard-library support block Vulkan learning; isolate compatibility helpers narrowly.

CMake recognizes `CXX_STANDARD 26` from version 3.25. `CXX_STANDARD_REQUIRED` is enabled so CMake may not silently decay to an older mode.
