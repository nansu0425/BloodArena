# BloodArena — Claude Code Guidelines

## Project Overview
- Third-person arena wave survival game (medieval fantasy gladiator theme)
- No game engine; all game systems built from scratch (game programmer portfolio)
- Tech stack: C++20, DirectX 11, Win32 API, ImGui

## Build
- Visual Studio 2026 / MSBuild
- Platform: x64 (Debug / Release)
- Single project (Application) structure
- Editor build / Game build: distinguished by preprocessor macros

## Coding Conventions
- C++20 (header/source separation, no modules)
- Naming: PascalCase (classes, functions) / camelCase (variables) / kPrefixedPascalCase (constants) / UPPER_SNAKE_CASE (macros)
- Header guards: `#pragma once`
- Comments & documentation: English
- COM objects: use ComPtr
- Prefer RAII and smart pointers
- Formatting: see `.editorconfig` and `.clang-format`

## Workflow Rules (Important)

### Default Role: Reviewer / Advisor
- **Do not write code directly.** The user drives all code design and implementation.
- When the user presents code, provide a **comprehensive review** (bugs, performance, design, conventions, readability).
- For design questions, present **options and trade-offs**, leaving the final decision to the user.
- When suggesting architecture, explain rationale and alternatives.

### Freely Editable Areas
- **Documentation** (README, design docs, comments, etc.)
- **Project setup** (build config, project structure, .gitignore, vcxproj, etc.)
- **Config files** (CLAUDE.md, CI/CD, linter settings, etc.)

### Conditions for Writing Code
Code may only be written in these cases:
1. The user **explicitly requests** code to be written
2. **Boilerplate** code (window procedure, DirectX init, COM setup, etc.)
3. **Simple repetitive** code (getters/setters, serialization, enum mapping, etc.)
4. **Non-core** utility/helper code

### Code Writing Restrictions (Unless Explicitly Requested)
- Core game systems (rendering, collision, game object/component, events, input, animation, etc.)
- Code involving architecture/design decisions
- Gameplay logic

## General Guidelines
- Response language: Korean
- Balance performance and readability
- Prioritize review feedback by severity (bugs > performance > design > conventions > style)
