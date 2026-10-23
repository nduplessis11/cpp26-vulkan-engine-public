# Coding standards

## Design

- Make ownership and lifetime obvious.
- Prefer value types and explicit composition.
- No owning raw pointers.
- Avoid hidden work in getters, constructors, and implicit conversions.
- Avoid giant manager classes and service locators.
- Keep interfaces narrow; do not abstract hypothetical backends.
- Use data-oriented layouts where measurement or access patterns justify them.

## C++

- Use `snake_case` for functions, variables, and files and `PascalCase` for types unless an ADR changes it.
- Declare functions with a trailing return type: `auto f(args) -> T`, including `main` and `void` returns (`auto f() -> void`). `.clang-tidy` runs `modernize-use-trailing-return-type`, which catches every case except `void` — the check has no option to include it, so `void` declarations are review-enforced only.
- Use `enum class`, `[[nodiscard]]`, `noexcept`, `std::span`, and `std::string_view` where semantics fit.
- Prefer scoped owners and explicit reset or release operations.
- Keep headers self-contained and minimize includes.
- Use `#pragma once`. `.clang-tidy` subtracts `portability-avoid-pragma-once` for this reason.
- Do not suppress a warning without documenting why the code is correct.
- Prefer `pre`, `post`, and `contract_assert` over regular `assert` for runtime invariants. Contract predicates must have no side effects. Keep `static_assert` for compile-time checks and GoogleTest assertions for test expectations. See [ADR 0010](adr/0010-prefer-language-contracts.md). Virtual functions cannot take `pre` or `post`; see "C++ interfaces" below.
- Avoid macros except narrow platform, compiler, logging, test, or generated metadata boundaries. Use language contracts directly in engine code.
- Run `make lint` before asking for a diff review. No test preset enables clang-tidy, so nothing else runs the checks in `.clang-tidy`. See `docs/BUILDING.md`.

## C++ interfaces

A C++ interface declares virtual operations that concrete derived types implement.
Declare every interface with `struct` and prefix its PascalCase name with `I`.
Every virtual method except the destructor must be pure virtual, declared with
`= 0`. Keep the public virtual destructor defaulted inside the struct.

Declare the default constructor and the copy and move operations `protected`
and `= default`. A defaulted virtual destructor does not remove the public
assignment operator that the compiler generates. Without the protected section,
`sink_a = sink_b` through two `ISink&` references compiles with no warning and
calls `ISink::operator=`. `ISink` has no data members, so that call copies
nothing and `sink_a` keeps its old contents. With the section, the line fails to
compile, and a derived type such as `CaptureSink` can still copy itself. The
default constructor is in the list because declaring a copy constructor removes
the implicit default constructor, and every derived type needs it.

```cpp
struct ISink {
    virtual ~ISink() = default;
    virtual auto write(LogRecord const&) -> void = 0;

  protected:
    ISink() = default;
    ISink(ISink const&) = default;
    ISink(ISink&&) = default;
    auto operator=(ISink const&) -> ISink& = default;
    auto operator=(ISink&&) -> ISink& = default;
};
```

The test file for each interface asserts this with
`static_assert(!std::is_copy_assignable_v<ISink> && !std::is_move_assignable_v<ISink>)`.
See `tests/core/logger_test.cpp` and `tests/platform/window_test.cpp`.

GCC 16 rejects `pre` and `post` on any virtual function, both on the
interface's pure virtual method and on the derived type's `override`:
`error: contracts cannot be added to virtual functions`. Check a caller
requirement on an interface operation with `contract_assert` at the start of
each implementation's body. Non-virtual functions, such as a concrete type's
`create` factory, still use `pre` and `post`.

`IWindow` follows the same convention. Concrete types such as `HeadlessWindow`,
`StderrSink`, and `CaptureSink` keep their existing names and implement operations
with `override`. Header filenames still use `snake_case`; `IWindow` stays in
`window.hpp`, and `ISink` stays in `logger.hpp`.

Apply this convention to curriculum examples as well as engine code. Check the
struct declaration, name, methods, destructor, protected section, and contracts
during review.

## Vulkan

- Check every `VkResult` that may fail.
- Name objects in debug builds once debug utils exists.
- Record parent and child destruction order.
- Keep feature chains and capability decisions inspectable.
- Store resource state explicitly.
- Never reuse a GPU-visible slot merely because the CPU object was destroyed.

## Commits

Each completed lesson should be one reviewable commit with its vertical-slice outcome, tests or evidence, progress update, and no unrelated churn.
