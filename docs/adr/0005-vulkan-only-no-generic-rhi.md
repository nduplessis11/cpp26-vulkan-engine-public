# ADR 0005: Vulkan-only renderer; no speculative generic RHI

- Status: Accepted
- Date: 2026-07-26

## Context

A cross-API abstraction would multiply design work before one backend is understood.

## Decision

Keep Vulkan details isolated for code health but expose contracts designed for this engine rather than hypothetical graphics APIs.

## Consequences

The design can use Vulkan directly. Another graphics API would require a future ADR and refactoring.
