# Testing strategy

## Directory layout

Organize module tests under `tests/<module>/`, matching the engine module name.
`tests/core/` contains the core tests and their helpers. `tests/platform/`
contains window and event tests. Add other module directories when tests need them.

Keep `tests/integration/` for tests spanning modules, `tests/gpu/` for GPU
validation, `tests/golden/` for image comparisons, and `tests/smoke/` for basic
executable checks. A module directory can contain unit tests; the folder name
does not determine the test category.

Register GoogleTest cases through `engine_add_gtest()` in `tests/CMakeLists.txt`.
CTest runs the registered cases.

## Test categories

- **Unit:** math, allocators, handles, parsers, ECS, serialization, render-graph compilation.
- **Integration:** platform event translation, VFS and registry, async pipelines, scene lifecycle.
- **GPU validation:** Vulkan creation, lifetimes, descriptor updates, synchronization, resize and recreation.
- **Golden images:** selected stable outputs with tolerances and inspectable diffs.
- **Stress:** resource churn, resize, minimize, reload, job shutdown, stale handles.
- **Manual captures:** RenderDoc, Nsight, or equivalent where automated proof is insufficient.

A screenshot proves appearance, not lifetime or synchronization. Unexpected validation messages fail relevant tests. GPU tests must distinguish unsupported environments from failures. Deterministic CPU algorithms should not require a GPU.
