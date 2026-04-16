# Blood Arena

A third-person arena wave survival game with a medieval fantasy gladiator theme, built from scratch in C++ without an existing game engine.

## Project Purpose

A **game programmer portfolio** — building a complete game from scratch in C++ / DirectX 11, without an existing game engine.

### What is built by hand vs. delegated to libraries

The boundary is **architectural control**. Code that decides how the game works — its structure, flow, and behavior — is written directly. Libraries are used only where they don't take over that control: they receive input, produce output, and the game code decides when and how to use the result.

- **Built by hand**: game loop, rendering pipeline, input, scene management, player controller, camera, AI, combat, progression, wave orchestration, UI flow, editor tooling
- **Libraries**: asset parsing, image/audio decoding, physics simulation, SIMD math, compression, serialization

This is why no existing game engine is used — engines impose their own architecture. See [CLAUDE.md](CLAUDE.md) for the full policy.

## Tech Stack

- C++20
- DirectX 11
- Windows API
- Dear ImGui (editor UI primitives)
- Third-party libraries for subsystem internals, managed via `vcpkg` manifest mode

## Build Configuration

Single-project (Application) structure. Two build variants are supported via preprocessor macros:

- **Editor Build** — Includes scene editing and debug UI, supports PIE (Play in Editor)
- **Game Build** — Runs in play mode without editor UI
