# ADR 0007: Own foundational engine systems

- Status: Accepted
- Date: 2026-07-26

## Context

The purpose is learning and ownership rather than fastest assembly.

## Decision

Write the math layer, GPU allocator exercises, ECS, render graph, small UI, basic physics, and basic audio path. Use external infrastructure only under the dependency policy.

## Consequences

Scope is large. Small vertical slices and strict deferred features are required.
