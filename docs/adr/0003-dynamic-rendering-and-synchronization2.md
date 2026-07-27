# ADR 0003: Dynamic rendering and synchronization2 are the normal path

- Status: Accepted
- Date: 2026-07-26

## Context

Legacy render passes, framebuffers, and older synchronization APIs are not required by the target baseline.

## Decision

Use dynamic rendering from the first clear and draw and synchronization2 for barriers and submissions. Do not build a parallel legacy architecture.

## Consequences

Older tutorials must be translated rather than copied. Attachment formats and states remain explicit.
