# Architecture

## Top-level ownership

Applications own the engine runtime. The runtime owns subsystems in declared order. Subsystems receive explicit references to required services. Avoid a service locator or global singleton graph.

```text
sandbox/editor
  -> runtime
       -> core services
       -> platform
       -> jobs
       -> assets
       -> renderer (Vulkan)
       -> scene/ECS
       -> audio/physics/etc.
```

This is direction, not permission to create every class now.

## Dependency direction

- `core`: diagnostics, contracts, clocks, IDs, and earned primitives.
- `math`: engine-owned math; standard library and core only.
- `platform`: native windows, events, and input normalization.
- `render`: renderer contracts and Vulkan backend.
- `assets`: IDs, VFS, registry, loading and cooking contracts; no Vulkan details.
- `ecs` and `scene`: simulation and scene representation; no direct Vulkan dependency.
- `editor`: consumes runtime contracts; never becomes a runtime dependency.
- `apps`: compose systems.

## Renderer policy

This is a Vulkan engine, not a generic RHI. The normal path uses Vulkan 1.4, dynamic rendering, synchronization2, timeline semaphores where useful, descriptor-indexing bindless, explicit state, deferred destruction, shared 2D and 3D infrastructure, and a render graph only after manual synchronization is understood.

## Data and lifetime

- CPU public handles are stable and generational where stale references matter.
- Raw Vulkan handles are backend details.
- Shader indices are not lifetime-safe CPU handles.
- Per-frame transient data comes from frame-owned arenas.
- GPU resources and descriptor slots retire against known completion values.
- Serialization stores stable IDs and schema versions, never pointers or transient GPU handles.

## Error policy

Use assertions for internal invariants, expected-style results for recoverable boundary failures, contextual errors at platform, Vulkan, asset, and tool boundaries, no exceptions across real-time or C ABI callbacks, and no swallowed Vulkan results.
