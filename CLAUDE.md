# BloodArena — Claude Code Guidelines

## Coding Conventions
- Headers: declarations only. Definitions go in `.cpp` files.
- Naming:
  - PascalCase: classes, functions, enums
  - camelCase: variables
  - Prefixes: `m_` (member), `s_` (static), `g_` (global), `k` (constant)
  - Macros: `BA_UPPER_SNAKE_CASE`
- All classes and functions must be defined within the `BA` namespace
- Code should be self-documenting. Avoid code that requires comments to be understood.
- Fail fast: use project crash/assert macros aggressively — surface problems early, never hide them.
- Formatting: see `.editorconfig` and `.clang-format`

## Workflow Rules

### Default Role: Coding Agent
- Claude acts as a **coding agent** that can write code in **all areas** (including core game systems, architecture, gameplay logic).
- All code must be **transparent** — the user must be able to understand and explain every line. Black-box code is not acceptable.

### Work Process (4 Steps)
1. **Explore** — Identify the scope of work and related code
2. **Plan** — Explain design intent and approach, including **why** each decision was made. Proceed only after user approval.
3. **Implement** — Write code according to the approved plan
4. **Review** — User reviews the code. Commit only when the user fully understands and is satisfied with the code.

### Build
- The user handles all builds manually. Claude must **not** run build commands.

### Transparency Principles
- If the user cannot understand or explain the code, it will not be used
- Plans must include rationale ("why this approach")
- Respond fully to user questions during code review

## Language
- Response: Korean
- Everything else (code, comments, documentation, commit messages, etc.): English
