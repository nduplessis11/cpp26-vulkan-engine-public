# ADR 0002: Vulkan 1.4 is the hard graphics baseline

- Status: Accepted
- Date: 2026-07-26

## Context

Older Vulkan support would add extension and fallback branches that distract from a modern curriculum.

## Decision

Require Vulkan 1.4 at loader and physical-device level. Keep feature and limit queries explicit even for promoted or mandatory functionality.

## Consequences

Older GPUs and drivers are unsupported. The engine gains a clearer modern baseline.
