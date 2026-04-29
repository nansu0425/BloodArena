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

Dependencies are managed via vcpkg manifest mode. The repository's solution-root `Directory.Build.props` / `Directory.Build.targets` explicitly import vcpkg's MSBuild integration, so the build is **IDE- and Installer-agnostic** — it does not depend on `vcpkg integrate install`'s user-wide files or on Visual Studio Installer's `vcpkg package manager` component.

### One-time setup

1. **Install Visual Studio 2026 Community** with the **Game development with C++** workload. Required optional components:
   - `MSVC Build Tools for x64/x86 (Latest)` — provides the v145 toolset that the project targets
   - `MSVC AddressSanitizer`
   - `Windows 11 SDK (latest)`
   - `HLSL Tools`

   Uncheck everything else — in particular **`vcpkg package manager`** (the project provides its own integration), `C++ profiling tools` (Tracy already covers in-engine profiling), `IntelliCode`, and all Unreal Engine related items. No additional Individual Components are required.

2. **Install the D3D11 Debug Layer.** Debug builds (`Debug_Editor` / `Debug_Game`) create the D3D11 device with `D3D11_CREATE_DEVICE_DEBUG`, which requires the OS-level "Graphics Tools" optional component (not bundled with Visual Studio or the Windows SDK). Run in **administrator PowerShell**:

   ```powershell
   Add-WindowsCapability -Online -Name "Tools.Graphics.DirectX~~~~0.0.1.0"
   ```

   GUI alternative: **Settings → Apps → Optional features → Add an optional feature → "Graphics Tools"**. No reboot required.

   Without this, debug builds crash on startup at D3D11 device creation with `DXGI_ERROR_SDK_COMPONENT_MISSING (0x887a002d)`. Release builds don't enable the debug flag and run without this component — but the debug layer is invaluable for catching D3D usage errors during development.

3. **Set up vcpkg.** From the repository root in PowerShell:

   ```powershell
   powershell -ExecutionPolicy Bypass -File Scripts\setup-env.ps1
   ```

   The script idempotently performs: vcpkg clone → bootstrap → `vcpkg integrate install` (user-wide; harmless with the self-contained approach) → `VCPKG_ROOT` user environment variable registration. Override the install path with `-VcpkgRoot C:\other\path` if needed.

4. **Restart Visual Studio** so it inherits the newly registered `VCPKG_ROOT`.

### How vcpkg integrates with the build

`Directory.Build.props` resolves the `VCPKG_ROOT` env var into the `VcpkgRoot` MSBuild property and imports `$(VcpkgRoot)\scripts\buildsystems\msbuild\vcpkg.props`. `Directory.Build.targets` imports the matching targets. MSBuild auto-discovers both at the solution root, so:

- Manifest auto-restore (`<VcpkgEnableManifest>true</VcpkgEnableManifest>` in `Application.vcxproj`) runs on every build.
- The custom triplet `x64-windows-noexcept` is applied automatically via the `vcpkg-custom-triplets/` overlay declared in `vcpkg-configuration.json` — no manual configuration required.
- The first build downloads and compiles `directxtk`, `spdlog`, `tinygltf`, `nlohmann-json`, `imgui` (with `dx11-binding` / `win32-binding` / `docking-experimental`), `imguizmo`, and `tracy` — several to ~15 minutes depending on the machine. Subsequent builds reuse the `vcpkg_installed/` cache.

## Build Configuration

Single-project (Application) structure. Two build variants are supported via preprocessor macros:

- **Editor Build** — Includes scene editing and debug UI, supports PIE (Play in Editor)
- **Game Build** — Runs in play mode without editor UI
