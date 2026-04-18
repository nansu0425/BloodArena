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

## Environment Setup

Dependencies are managed via vcpkg manifest mode. On a fresh machine, run the one-time setup below before the first build.

From the repository root in PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File Scripts\setup-env.ps1
```

The script idempotently performs: vcpkg clone → bootstrap → `integrate install` → `VCPKG_ROOT` user environment variable registration. Override the install path with `-VcpkgRoot C:\other\path` if needed.

- `vcpkg integrate install`: applies user-wide MSBuild integration.
- `VCPKG_ROOT`: referenced by Visual Studio's MSBuild vcpkg integration for automatic manifest restore. Visual Studio must be fully restarted after the variable is set.
- The custom triplet `x64-windows-noexcept` is applied automatically via the `vcpkg-custom-triplets/` overlay declared in `vcpkg-configuration.json` — no manual configuration required.
- The first build downloads and compiles `directxtk`, `spdlog`, `tinygltf`, `nlohmann-json`, and `imgui`, which can take several minutes. Subsequent builds reuse the `vcpkg_installed/` cache.

## Build Configuration

Single-project (Application) structure. Two build variants are supported via preprocessor macros:

- **Editor Build** — Includes scene editing and debug UI, supports PIE (Play in Editor)
- **Game Build** — Runs in play mode without editor UI
