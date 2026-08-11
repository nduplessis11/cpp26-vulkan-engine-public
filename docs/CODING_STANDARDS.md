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
- Use `#pragma once`.
- Do not suppress a warning without documenting why the code is correct.
- Avoid macros except narrow platform, compiler, logging, assertion, or generated metadata boundaries.

## Vulkan

- Check every `VkResult` that may fail.
- Name objects in debug builds once debug utils exists.
- Record parent and child destruction order.
- Keep feature chains and capability decisions inspectable.
- Store resource state explicitly.
- Never reuse a GPU-visible slot merely because the CPU object was destroyed.

## Commits

Each completed lesson should be one reviewable commit with its vertical-slice outcome, tests or evidence, progress update, and no unrelated churn.
