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

## Spec-Driven Development

### Spec-as-Source
- Specs in `Documentation/Specs/` are the source of truth for all implementation.
- Code depends on specs. The reverse is not allowed.
- Flow: spec → code. Never: code → spec.
- When a spec changes, the corresponding code must be updated to reflect it.
- Code must not be modified independently of its spec.
- When spec and code disagree, the spec is correct and the code must be fixed.
- If the spec itself is wrong, update the spec first via `/spec-update`, then fix the code.

### Spec Structure
- Specs are organized by module, mirroring the code structure under `Application/`.
- Each module can contain multiple specs.
- Path format: `Documentation/Specs/{Module}/{feature}/`

### Workflow
- `/spec-create {description}`: Create a new feature spec. Module placement is decided during design.
- `/spec-implement {Module/feature} {task#}`: Implement a single task from an existing spec.
- `/spec-update {Module/feature}`: Update a spec and propagate changes to code.
- Without an active command, Claude operates as a general assistant.

## Build
- The user handles all builds manually. Claude must **not** run build commands.

## References
- Project roadmap: `Documentation/ROADMAP.md`
- Feature specs: `Documentation/Specs/{Module}/{feature}/`

## Language
- Response: Korean
- Everything else (code, comments, documentation, commit messages, etc.): English
