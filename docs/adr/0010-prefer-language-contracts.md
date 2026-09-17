# ADR 0010: Prefer language contracts for runtime invariants

- Status: Accepted
- Date: 2026-09-13

The learner requested migration of existing assertions from lessons 000–005 and
replacement of the rule that preferred `assert`. Use `pre` for caller
requirements, `post` for guarantees on return, and `contract_assert` for checks
inside a function. A predicate is the Boolean expression in a contract. Predicates
must have no side effects; program behavior must not depend on their evaluation.
Keep `static_assert` for compile-time checks and GoogleTest assertions for tests.
Keep `std::expected` for recoverable boundary failures.

This decision supersedes the portable-`assert` preference in earlier teaching
notes. GCC implements contracts from the C++26 working draft; the earlier claim
that contracts were removed from that draft in 2024 was incorrect. GCC's
implementation remains experimental. See the
[GCC contracts documentation](https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html#index-fcontracts).

## Migration design

- **Owner:** `version_component` checks its caller's index. `FakeHandle::get`
  checks the handle state. Neither check owns a resource.
- **Inputs:** A signed index and the handle's existing `is_valid()` result.
- **Outputs:** The selected version field and the owned handle ID.
- **Invariants:** `0 <= index <= 2`; `get()` requires a valid handle.
- **Failure path:** Enforced violations invoke GCC's default handler and terminate.
  They do not pass through the logger or return an error value.
- **Proof:** Existing success tests and three death tests exercise these checks.
  Verify the Linux Debug and sanitizer suites, MinGW compilation, a non-Debug
  build, and Clang tooling. Lesson 005 has no existing assertions to migrate.

## Build and tooling policy

GCC must compile and link with `-fcontracts`. CMake checks the feature-test macro
and compiles a sample with all three contract forms before enabling the flags.
Debug uses `enforce`; other configurations use `ignore`. This preserves the
existing Debug-only runtime checking policy. `NDEBUG` does not control contracts.

`ENGINE_ENABLE_CONTRACTS=OFF` is reserved for the configure-only Clang tooling
preset. It does not produce runnable engine binaries. GCC contract support is a
build requirement; there is no source-level fallback to `assert`.

The pinned MinGW GCC 17 library leaves its weak default-handler symbol
unresolved, including with static linking and `--whole-archive`. When the normal
probe fails on MinGW, CMake probes a strong forwarding definition from
`cmake/compat/mingw_contract_handler.cpp`. If that links, each executable compiles
the forwarding definition. It calls
`std::contracts::invoke_default_contract_violation_handler` and adds no logging
policy. Toolchains that pass the normal probe do not use this workaround.

The `.clangd` configuration and `make lint` erase contract clauses with macros.
Those tools cannot check contract predicates. Calls to unrelated functions named
`pre` or `post` also collide with these macros. GCC remains the authority for
contract diagnostics. Recorded learner answers retain their original wording;
policy corrections are marked separately.

## Verification

- `make test-docker-linux`: 44 tests passed on GCC 17, including all three
  contract death tests.
- `make test-docker-linux-asan`: the same 44 tests passed with AddressSanitizer
  and UndefinedBehaviorSanitizer.
- `make build-docker-windows`: all targets linked with the MinGW workaround.
  `engine_version_test.exe` and `engine_fake_handle_test.exe` passed all 25 tests
  under Wine in a temporary prefix, including the three death tests. This was
  not a test on a native Windows host.
- A host GCC 16.2.1 Release configure in `out/build/linux-gcc-contracts-release`
  built both applications and ran the sandbox. A separate probe confirmed that
  `FakeHandle::get` terminates on an empty handle under `enforce` and returns
  zero under `ignore`.
- The forwarding handler produced the same failed predicate diagnostic in a
  native probe. `make lint`, clangd 22.1.8 checks of both affected test files
  and the sandbox, and `python tools/validate_curriculum.py` passed.
