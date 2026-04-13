# BloodArena — Claude Code Guidelines

## Coding Conventions
- Header/definition split: by default, headers contain only declarations and function bodies live in `.cpp` files. Exceptions — these MUST be defined in headers because the language requires the definition to be visible at the point of use:
  - `constexpr` functions and variables (must be visible for compile-time evaluation at call sites, including default member initializers and other `constexpr` contexts)
  - `inline` variables and `inline` functions (the `inline` keyword exists precisely to allow a single definition shared across translation units from a header)
  - Function and class templates (definitions required at instantiation)
  - Struct/class declarations themselves, including data member default initializers (the type layout is part of the declaration)
  Everything else — normal function bodies, non-`inline` globals, static member variable definitions — goes in `.cpp`. Do not put a regular function body in a header just because it is short.
- Naming:
  - PascalCase: classes, functions, enums
  - camelCase: variables
  - Prefixes: `m_` (class private members), `s_` (static), `g_` (global), `k` (constant)
  - Struct fields: plain camelCase (no `m_` prefix)
  - Macros: `BA_UPPER_SNAKE_CASE`
- All classes and functions must be defined within the `BA` namespace
- File-local helpers (non-member functions, file-scope variables, file-only types) must be placed inside an anonymous namespace, not marked `static`. Anonymous namespace applies uniformly to functions, variables, and types.
- Code should be self-documenting. Avoid code that requires comments to be understood.
- Fail fast: use project crash/assert macros aggressively — surface problems early, never hide them.
- Simple and intuitive code. Clarity over cleverness.
- No premature optimization. Only optimize verified bottlenecks.
- Minimize abstraction. Keep related code together. Added complexity must justify lost readability.
- Consistency over local convenience. A concept (sign convention, coordinate system, unit, direction) must mean the same thing everywhere it appears. Do not introduce local sign flips, inversions, or reinterpretations at call sites to match external habits or UI intuition — instead, align the stored representation with the underlying library/convention and document it once at the declaration. If UI-facing display needs a different form, convert at the display boundary only.

## Build
- The user handles all builds manually. Claude must **not** run build commands.

## References
- Project roadmap: `Documentation/ROADMAP.md`

## Language
- Response: Korean
- Everything else (code, comments, documentation, commit messages, etc.): English
