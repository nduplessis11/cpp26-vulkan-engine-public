# ADR 0006: Native Wayland and Win32 with XCB compatibility

- Status: Accepted
- Date: 2026-07-26

## Context

The learner wants to write the platform layer and support Linux and Windows. A toolkit would hide lifecycle, input, and WSI concepts.

## Decision

Implement Wayland first on Linux, Win32 on Windows, and XCB later as X11 compatibility. Keep a headless backend for tests.

## Consequences

The curriculum is longer and platform differences stay visible. There is no permanent SDL or GLFW dependency.
