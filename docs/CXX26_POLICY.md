# C++26 policy

The repository requests C++26 with CMake and requires the requested mode. This does not mean every C++26 language and library feature is available or equally stable on every supported compiler.

1. Build in C++26 or latest mode.
2. Check features against the active GCC Linux and MinGW build targets. Clang tools analyze code; they are not engine build targets.
3. Check feature-test macros and compiler status before adopting a C++26-only facility.
4. Introduce modules, contracts, reflection-related work, or major new library facilities through a small experiment and ADR.
5. Prefer clarity over novelty.
6. Do not let incomplete standard-library support block Vulkan learning; isolate compatibility helpers narrowly.

CMake recognizes `CXX_STANDARD 26` from version 3.25. `CXX_STANDARD_REQUIRED` is enabled so CMake may not silently decay to an older mode.

[ADR 0010](adr/0010-prefer-language-contracts.md) adopts language contracts for
runtime invariants. Prefer `pre`, `post`, and `contract_assert` over regular
`assert`. GCC parses contracts in every build configuration. Debug enforces
them; other configurations ignore them. Clang tooling erases the clauses and
cannot check their predicates. `static_assert` and test assertions keep their
existing roles.
