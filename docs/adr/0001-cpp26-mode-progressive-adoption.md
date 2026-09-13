# ADR 0001: C++26 mode with progressive feature adoption

- Status: Accepted
- Date: 2026-07-26

## Context

The project is explicitly C++26 while compiler and standard-library conformance differs across Clang, GCC, and MSVC.

## Decision

Compile all engine targets in required C++26 or latest mode. Adopt individual C++26-only facilities only after a feature-test or small cross-compiler experiment. Mature C++20 and C++23 facilities remain welcome.

## Consequences

The codebase honestly targets C++26 without making novelty a dependency. Some facilities may be deferred until supported toolchains agree.

[ADR 0010](0010-prefer-language-contracts.md) adopts language contracts on the
active GCC Linux and MinGW build targets after a feature probe and build checks.
Clang tooling erases contract clauses for analysis.
