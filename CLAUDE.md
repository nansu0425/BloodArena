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
  - Booleans (variables, fields, return values) must use an interrogative form so the name reads as a yes/no question. Use `is`/`has`/`can`/`should` prefixes (e.g. `isHit`, `hasValue`, `canMove`, `shouldRender`). Bare nouns/adjectives like `hit`, `ready`, `valid` are not allowed for `bool`.
- Function return style:
  - Function parameters are **input only**. Output parameters (`T&`/`T*` used to return values, in/out params) are not allowed. All results must come back through the return value.
  - `std::optional` is not used in this project. For "single value that may fail," return a small struct with an `is`-prefixed boolean field plus the value (e.g. `struct RayTriangleHit { bool isHit; float t; };`).
  - When multiple values must be returned together, define a named struct with descriptive fields. Do not use `std::tuple`/`std::pair` for return values — field names must carry the meaning.
- All classes and functions must be defined within the `BA` namespace
- File-local helpers (non-member functions, file-scope variables, file-only types) must be placed inside an anonymous namespace, not marked `static`. Anonymous namespace applies uniformly to functions, variables, and types.
- Minimize lambda use. Lambdas are only for one-line expressions such as simple predicates or projections passed to STL algorithms. For any multi-line helper, define a named function in the anonymous namespace and pass the needed state through parameters instead of capturing. Named functions aid code navigation, reuse, and debugging.
- Code should be self-documenting. Avoid code that requires comments to be understood.
- Minimize magic numbers. Any numeric literal that carries meaning (tolerances, thresholds, limits, tuning parameters, conversion factors) must be given a `k`-prefixed named constant at the narrowest appropriate scope (function-local `constexpr` when used in one function, file-local in an anonymous namespace when shared within a translation unit, header-level when shared across files). The name must convey the semantic role, not just the value. When two literals happen to share the same value but have different meanings, give them separate names so each can be tuned independently. Exceptions: trivially self-evident literals like `0`, `1`, `-1`, array indices, and loop bounds with obvious meaning.
- Single source of truth for default values. When a default value (initial setting, threshold, limit, etc.) is used in more than one place, it must be defined exactly once and every consumer must refer to that definition. Do not duplicate the same default in multiple locations (e.g. a settings struct and a class member). Instead, have one authoritative definition and derive the rest from it.
- Fail fast: use project crash/assert macros aggressively — surface problems early, never hide them.
- Simple and intuitive code. Clarity over cleverness.
- No premature optimization. Only optimize verified bottlenecks.
- Minimize abstraction. Keep related code together. Added complexity must justify lost readability.
- Consistency over local convenience. A concept (sign convention, coordinate system, unit, direction) must mean the same thing everywhere it appears. Do not introduce local sign flips, inversions, or reinterpretations at call sites to match external habits or UI intuition — instead, align the stored representation with the underlying library/convention and document it once at the declaration. If UI-facing display needs a different form, convert at the display boundary only.

## Project Goal and Scope
- **Game programmer portfolio** — build a complete game from scratch in C++ without an existing game engine.
- The boundary between hand-written code and libraries is **architectural control**. Code that decides how the game works (structure, flow, behavior) is written directly. Libraries are used only where they don't take over that control — they receive input, produce output, and the game code decides when and how to use the result. This is why no game engine is used: engines impose their own architecture.
- **Built by hand**: game loop, rendering pipeline, input handling, scene management, player controller, camera, AI, combat, progression, wave orchestration, UI flow, editor tooling.
- **Libraries**: asset parsing, physics simulation, image/audio decoding, font rasterization, SIMD math, compression, serialization, GUI primitives.
- **Rendering techniques** (shader math, lighting models, shadow algorithms, post-processing) are public recipes — translate from references. The pipeline code that integrates them is built by hand.

## External Dependencies
- Libraries are used for subsystem internals that don't affect architectural control. Specific choices are deferred until actually needed.
- **Wrapper layer is mandatory.** Every third-party library is accessed through a `BA::`-prefixed facade. Raw third-party types must not leak into game code.
- Dependencies are managed via `vcpkg` manifest mode. Prefer: permissive license (MIT/BSD/zlib/Apache-2.0), small integration footprint, actively maintained, minimal transitive dependencies.

## Build
- The user handles all builds manually. Claude must **not** run build commands.
- IDE diagnostics (IntelliSense errors/warnings) may be stale or incorrect. Do not treat them as ground truth or change code solely to satisfy them. Only actual build output from the compiler is authoritative.

## References
- Project roadmap: `Documentation/ROADMAP.md`

## Language
- Response: Korean
- Everything else (code, comments, documentation, commit messages, etc.): English
