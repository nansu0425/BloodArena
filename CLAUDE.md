# BloodArena — Claude Code Guidelines

## Coding Conventions
- Headers: declarations only. Definitions go in `.cpp` files.
- Naming:
  - PascalCase: classes, functions, enums
  - camelCase: variables
  - Prefixes: `m_` (class private members), `s_` (static), `g_` (global), `k` (constant)
  - Struct fields: plain camelCase (no `m_` prefix)
  - Macros: `BA_UPPER_SNAKE_CASE`
- All classes and functions must be defined within the `BA` namespace
- Code should be self-documenting. Avoid code that requires comments to be understood.
- Fail fast: use project crash/assert macros aggressively — surface problems early, never hide them.
- Simple and intuitive code. Clarity over cleverness.
- No premature optimization. Only optimize verified bottlenecks.
- Minimize abstraction. Keep related code together. Added complexity must justify lost readability.

## Design-Code Dependency
- Design documents and code are both first-class project resources.
- Code depends on design documents. The reverse is not allowed.
- Flow: design document → code. Never: code → design document.
- When a design document changes, the corresponding code must be updated to reflect it.
- Code must not be modified independently of its design document.

## Workflow
- Two work modes: `/design` (design) and `/implement` (implementation).
- Each mode has its own behavioral rules defined in the corresponding skill.
- Without an active mode, Claude operates as a general assistant.

## Build
- The user handles all builds manually. Claude must **not** run build commands.

## References
- Project roadmap: `Documentation/ROADMAP.md`
- Phase work orders: `Documentation/PHASE*.md`

## Language
- Response: Korean
- Everything else (code, comments, documentation, commit messages, etc.): English
