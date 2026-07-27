# ADR 0004: Descriptor indexing is the primary bindless model

- Status: Accepted
- Date: 2026-07-26

## Context

The engine needs bindless resources. Descriptor buffers and newer descriptor-heap work are promising but add portability and tooling risk.

## Decision

Begin with update-after-bind descriptor-indexing tables, non-uniform indexing, fallback slots, stable CPU generational handles, and deferred slot reuse. Evaluate descriptor buffers and heaps later in isolated experiments.

## Consequences

The first bindless design works across a broader Vulkan 1.4 ecosystem while preserving a migration path behind resource handles.
