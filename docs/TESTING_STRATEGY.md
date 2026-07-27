# Testing strategy

- **Unit:** math, allocators, handles, parsers, ECS, serialization, render-graph compilation.
- **Integration:** platform event translation, VFS and registry, async pipelines, scene lifecycle.
- **GPU validation:** Vulkan creation, lifetimes, descriptor updates, synchronization, resize and recreation.
- **Golden images:** selected stable outputs with tolerances and inspectable diffs.
- **Stress:** resource churn, resize, minimize, reload, job shutdown, stale handles.
- **Manual captures:** RenderDoc, Nsight, or equivalent where automated proof is insufficient.

A screenshot proves appearance, not lifetime or synchronization. Unexpected validation messages fail relevant tests. GPU tests must distinguish unsupported environments from failures. Deterministic CPU algorithms should not require a GPU.
